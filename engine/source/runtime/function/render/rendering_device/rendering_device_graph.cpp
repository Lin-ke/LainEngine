#include "rendering_device_graph.h"
// 生成后面的会导致vulkan被包含，隔离失败了
#ifdef PRINT_RESOURCE_TRACKER_TOTAL
#include "core/string/print_string.h"
#endif
#define PRINT_RENDER_GRAPH 1
#define PRINT_COMMAND_RECORDING 1
using namespace lain;
// 主要分为read和write两种
RenderingDeviceGraph::RenderingDeviceGraph() {}
RenderingDeviceGraph::~RenderingDeviceGraph() {}

uint32_t RenderingDeviceGraph::resource_tracker_total = 0;
bool RenderingDeviceGraph::_is_write_usage(ResourceUsage p_usage) {
  switch (p_usage) {
    case RESOURCE_USAGE_COPY_FROM:
    case RESOURCE_USAGE_RESOLVE_FROM:
    case RESOURCE_USAGE_UNIFORM_BUFFER_READ:
    case RESOURCE_USAGE_INDIRECT_BUFFER_READ:
    case RESOURCE_USAGE_TEXTURE_BUFFER_READ:
    case RESOURCE_USAGE_STORAGE_BUFFER_READ:
    case RESOURCE_USAGE_VERTEX_BUFFER_READ:
    case RESOURCE_USAGE_INDEX_BUFFER_READ:
    case RESOURCE_USAGE_TEXTURE_SAMPLE:
    case RESOURCE_USAGE_STORAGE_IMAGE_READ:
      return false;
    case RESOURCE_USAGE_COPY_TO:
    case RESOURCE_USAGE_RESOLVE_TO:
    case RESOURCE_USAGE_TEXTURE_BUFFER_READ_WRITE:
    case RESOURCE_USAGE_STORAGE_BUFFER_READ_WRITE:
    case RESOURCE_USAGE_STORAGE_IMAGE_READ_WRITE:
    case RESOURCE_USAGE_ATTACHMENT_COLOR_READ_WRITE:
    case RESOURCE_USAGE_ATTACHMENT_DEPTH_STENCIL_READ_WRITE:
      return true;
    default:
      DEV_ASSERT(false && "Invalid resource tracker usage.");
      return false;
  }
}

RDD::TextureLayout RenderingDeviceGraph::_usage_to_image_layout(ResourceUsage p_usage) {
  switch (p_usage) {
    case RESOURCE_USAGE_COPY_FROM:
      return RDD::TEXTURE_LAYOUT_COPY_SRC_OPTIMAL;
    case RESOURCE_USAGE_COPY_TO:
      return RDD::TEXTURE_LAYOUT_COPY_DST_OPTIMAL;
    case RESOURCE_USAGE_RESOLVE_FROM:
      return RDD::TEXTURE_LAYOUT_RESOLVE_SRC_OPTIMAL;
    case RESOURCE_USAGE_RESOLVE_TO:
      return RDD::TEXTURE_LAYOUT_RESOLVE_DST_OPTIMAL;
    case RESOURCE_USAGE_TEXTURE_SAMPLE:
      return RDD::TEXTURE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case RESOURCE_USAGE_STORAGE_IMAGE_READ:  // image read, 和 image read-write都是 storage optimal
    case RESOURCE_USAGE_STORAGE_IMAGE_READ_WRITE:
      return RDD::TEXTURE_LAYOUT_STORAGE_OPTIMAL;
    case RESOURCE_USAGE_ATTACHMENT_COLOR_READ_WRITE:
      return RDD::TEXTURE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case RESOURCE_USAGE_ATTACHMENT_DEPTH_STENCIL_READ_WRITE:
      return RDD::TEXTURE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case RESOURCE_USAGE_NONE:
      return RDD::TEXTURE_LAYOUT_UNDEFINED;
    default:
      DEV_ASSERT(false && "Invalid resource tracker usage or not an image usage.");
      return RDD::TEXTURE_LAYOUT_UNDEFINED;
  }
}

RDD::BarrierAccessBits RenderingDeviceGraph::_usage_to_access_bits(ResourceUsage p_usage) {
#if FORCE_FULL_ACCESS_BITS
  return RDD::BarrierAccessBits(RDD::BARRIER_ACCESS_MEMORY_READ_BIT |
                                RDD::BARRIER_ACCESS_MEMORY_WRITE_BIT);
#else
  switch (p_usage) {
    case RESOURCE_USAGE_NONE:
      return RDD::BarrierAccessBits(0);
    case RESOURCE_USAGE_COPY_FROM:
      return RDD::BARRIER_ACCESS_COPY_READ_BIT;
    case RESOURCE_USAGE_COPY_TO:
      return RDD::BARRIER_ACCESS_COPY_WRITE_BIT;
    case RESOURCE_USAGE_RESOLVE_FROM:
      return RDD::BARRIER_ACCESS_RESOLVE_READ_BIT;
    case RESOURCE_USAGE_RESOLVE_TO:
      return RDD::BARRIER_ACCESS_RESOLVE_WRITE_BIT;
    case RESOURCE_USAGE_UNIFORM_BUFFER_READ:
      return RDD::BARRIER_ACCESS_UNIFORM_READ_BIT;
    case RESOURCE_USAGE_INDIRECT_BUFFER_READ:
      return RDD::BARRIER_ACCESS_INDIRECT_COMMAND_READ_BIT;
    case RESOURCE_USAGE_STORAGE_BUFFER_READ:
    case RESOURCE_USAGE_STORAGE_IMAGE_READ:
    case RESOURCE_USAGE_TEXTURE_BUFFER_READ:
    case RESOURCE_USAGE_TEXTURE_SAMPLE:
      return RDD::BARRIER_ACCESS_SHADER_READ_BIT;
    case RESOURCE_USAGE_TEXTURE_BUFFER_READ_WRITE:
    case RESOURCE_USAGE_STORAGE_BUFFER_READ_WRITE:
    case RESOURCE_USAGE_STORAGE_IMAGE_READ_WRITE:
      return RDD::BarrierAccessBits(RDD::BARRIER_ACCESS_SHADER_READ_BIT |
                                    RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
    case RESOURCE_USAGE_VERTEX_BUFFER_READ:
      return RDD::BARRIER_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    case RESOURCE_USAGE_INDEX_BUFFER_READ:
      return RDD::BARRIER_ACCESS_INDEX_READ_BIT;
    case RESOURCE_USAGE_ATTACHMENT_COLOR_READ_WRITE:
      return RDD::BarrierAccessBits(RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                    RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
    case RESOURCE_USAGE_ATTACHMENT_DEPTH_STENCIL_READ_WRITE:
      return RDD::BarrierAccessBits(RDD::BARRIER_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                    RDD::BARRIER_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
    default:
      DEV_ASSERT(false && "Invalid usage.");
      return RDD::BarrierAccessBits(0);
  }
#endif
}

void RDG::_add_command_to_graph(ResourceTracker** p_resource_trackers,
                                ResourceUsage* p_resource_usages, uint32_t p_resource_count,
                                int32_t p_command_index, RecordedCommand* r_command) {
  // Assign the next stages derived from the stages the command requires first.
  r_command->next_stages = r_command->self_stages;
  if (command_label_index >= 0) {
    // If a label is active, tag the command with the label.
    r_command->label_index = command_label_index;
  }
  // timestamp和synchronization是特殊的command，需要单独处理(需要将之前的命令与该命令相连)
  if (r_command->type == RecordedCommand::TYPE_CAPTURE_TIMESTAMP) {
    // All previous commands starting from the previous timestamp should be adjacent to this command.
    int32_t start_command_index = uint32_t(MAX(command_timestamp_index, 0));
    for (int32_t i = start_command_index; i < p_command_index; i++) {
      _add_adjacent_command(i, p_command_index, r_command);
    }

    // Make this command the new active timestamp command.
    command_timestamp_index = p_command_index;
  } else if (command_timestamp_index >= 0) {
    // Timestamp command should be adjacent to this command.
    _add_adjacent_command(command_timestamp_index, p_command_index, r_command);
  }
  // 处于synchronization的状态
  if (command_synchronization_pending) {
    // All previous commands should be adjacent to this command.
    int32_t start_command_index = uint32_t(MAX(command_synchronization_index, 0));
    for (int32_t i = start_command_index; i < p_command_index; i++) {
      _add_adjacent_command(i, p_command_index, r_command);
    }

    command_synchronization_index = p_command_index;
    command_synchronization_pending = false;
  } else if (command_synchronization_index >= 0) {
    // Synchronization command should be adjacent to this command.
    _add_adjacent_command(command_synchronization_index, p_command_index, r_command);
  }

  for (uint32_t i = 0; i < p_resource_count; i++) {
    ResourceTracker* resource_tracker = p_resource_trackers[i];
    DEV_ASSERT(resource_tracker != nullptr);
    resource_tracker->reset_if_outdated(tracking_frame);
    const Rect2i resource_tracker_rect = resource_tracker->get_subresources();

    Rect2i search_tracker_rect = resource_tracker_rect;

    ResourceUsage new_resource_usage = p_resource_usages[i];
    bool write_usage = _is_write_usage(new_resource_usage);
    BitField<RDD::BarrierAccessBits> new_usage_access = _usage_to_access_bits(new_resource_usage);
    ResourceTracker* parent_traker = resource_tracker->parent;
    bool is_resource_a_slice = parent_traker != nullptr;
    if (is_resource_a_slice) {
      // This resource depends on a parent resource.
      parent_traker->reset_if_outdated(tracking_frame);
      if (resource_tracker->slice_cmd_idx != p_command_index) {
        // Indicate this slice has been used by this command.
        resource_tracker->slice_cmd_idx = p_command_index;
      }
      if (parent_traker->usage == ResourceUsage::RESOURCE_USAGE_NONE) {
        if (parent_traker->is_texture()) {  // 需要转换parent的layout
          // If the resource is a texture, we transition it entirely to the layout determined by the first slice that uses it.
          _add_texture_barrier_to_command(
              parent_traker->texture_driver_id, RDD::BarrierAccessBits(0), new_usage_access,
              RDG::RESOURCE_USAGE_NONE, new_resource_usage, parent_traker->texture_subresources,
              command_normalization_barriers, r_command->normalization_barrier_index,
              r_command->normalization_barrier_count);
        }
        // If the parent hasn't been used yet, we assign the usage of the slice to the entire resource.
        parent_traker->usage = new_resource_usage;

        // Also assign the usage to the slice and consider it a write operation. Consider the parent's current usage access as its own.
        resource_tracker->usage = new_resource_usage;
        resource_tracker->usage_access = parent_traker->usage_access;
        write_usage = true;
        // Indicate the area that should be tracked is the entire resource.
        search_tracker_rect = parent_traker->get_subresources();
      }  // if (usage == None)
      else if (resource_tracker->in_parent_dirty_list) {
        if (parent_traker->usage == new_resource_usage) {
          // The slice will be transitioned to the resource of the parent and can be deleted from the dirty list.
          ResourceTracker* previous_tracker = nullptr;
          ResourceTracker* current_tracker = parent_traker->dirty_shared_list;
          bool initialized_dirty_rect = false;
          while (current_tracker != nullptr) {  //  找到 resource_tracker
            current_tracker->reset_if_outdated(tracking_frame);

            if (current_tracker == resource_tracker) {
              current_tracker->in_parent_dirty_list = false;

              if (previous_tracker != nullptr) {
                previous_tracker->next_shared = current_tracker->next_shared;
              } else {
                resource_tracker->parent->dirty_shared_list = current_tracker->next_shared;
              }

              current_tracker = current_tracker->next_shared;
            } else {
              if (initialized_dirty_rect) {  // 非resource_tracker的重新融合一遍
                parent_traker->slice_or_dirty_rect =
                    parent_traker->slice_or_dirty_rect.merge(current_tracker->slice_or_dirty_rect);
              } else {
                parent_traker->slice_or_dirty_rect = current_tracker->slice_or_dirty_rect;
                initialized_dirty_rect = true;
              }

              previous_tracker = current_tracker;
              current_tracker = current_tracker->next_shared;
            }
          }
        }
      }  // if (in parent dirty)
      else {  // not in parent dirty list
        if (parent_traker->dirty_shared_list != nullptr &&
            parent_traker->slice_or_dirty_rect.intersects(resource_tracker->slice_or_dirty_rect)) {

          // There's an intersection with the current dirty area of the parent and the slice. We must verify if the intersection is against a slice
          // that was used in this command or not. Any slice we can find that wasn't used by this command must be reverted to the layout of the parent.
          ResourceTracker* _prev = nullptr;
          ResourceTracker* _curr = parent_traker->dirty_shared_list;
          bool initialized_dirty_rect = false;
          while (_curr != nullptr) {
            _curr->reset_if_outdated(tracking_frame);
            if (_curr->slice_or_dirty_rect.intersects(resource_tracker->slice_or_dirty_rect)) {
              if (_curr->slice_cmd_idx == p_command_index) {
                ERR_FAIL_MSG("Texture slices that overlap can't be used in the same command.");
              } else {
                // 将与该slice有交集的slice恢复为父纹理的使用
                //如果新命令想要使用与脏区域重叠的切片，脏列表中存在的切片也可以单独进行规范化和删除。
                // Delete the slice from the dirty list and revert it to the usage of the parent.
                if (_curr->is_texture()) {
                  _add_texture_barrier_to_command(
                      _curr->texture_driver_id, _curr->usage_access, new_usage_access, _curr->usage,
                      parent_traker->usage, _curr->texture_subresources,
                      command_normalization_barriers, r_command->normalization_barrier_index,
                      r_command->normalization_barrier_count);
                  search_tracker_rect =
                      search_tracker_rect.merge(_curr->slice_or_dirty_rect);  // 受到影响的部分
                  write_usage = true;
                }
                _curr->in_parent_dirty_list = false;
                if (_prev != nullptr) {
                  _prev->next_shared = _curr->next_shared;
                } else {
                  parent_traker->dirty_shared_list = _curr->next_shared;
                }
                _curr = _curr->next_shared;
              }
            }  // if intersect
            else {
              // Recalculate the dirty rect of the parent so the deleted slices are excluded.
              if (initialized_dirty_rect) {
                resource_tracker->parent->slice_or_dirty_rect =
                    resource_tracker->parent->slice_or_dirty_rect.merge(_curr->slice_or_dirty_rect);
              } else {
                resource_tracker->parent->slice_or_dirty_rect = _curr->slice_or_dirty_rect;
                initialized_dirty_rect = true;
              }
              _prev = _curr;
              _curr = _curr->next_shared;
            }
          }
        }
        resource_tracker->usage = parent_traker->usage;
        resource_tracker->usage_access = parent_traker->usage_access;

        if (resource_tracker->usage != new_resource_usage) {
          // Insert to the dirty list if the requested usage is different.
          resource_tracker->next_shared = parent_traker->dirty_shared_list;
          parent_traker->dirty_shared_list = resource_tracker;
          resource_tracker->in_parent_dirty_list = true;
          if (parent_traker->dirty_shared_list != nullptr) {
            parent_traker->slice_or_dirty_rect =
                parent_traker->slice_or_dirty_rect.merge(resource_tracker->slice_or_dirty_rect);
          } else {
            parent_traker->slice_or_dirty_rect = resource_tracker->slice_or_dirty_rect;
          }
        }
      }
    } else {  // not slice
      //如果在命令中直接使用父纹理，则脏列表中的所有切片都将通过将纹理“规范化”为一个内存布局而恢复为父纹理的使用。
      ResourceTracker* current_tracker = resource_tracker->dirty_shared_list;
      if (current_tracker != nullptr) {
        // Consider the usage as write if we must transition any of the slices.
        write_usage = true;
      }

      while (current_tracker != nullptr) {
        current_tracker->reset_if_outdated(tracking_frame);
        // 脏列表一定与父纹理的layout不同
        if (current_tracker->is_texture()) {
          DEV_ASSERT(_usage_to_image_layout(current_tracker->usage) ==
                     _usage_to_image_layout(new_resource_usage));
          // Transition all slices to the layout of the parent resource.
          _add_texture_barrier_to_command(
              current_tracker->texture_driver_id, current_tracker->usage_access, new_usage_access,
              current_tracker->usage, resource_tracker->usage,
              current_tracker->texture_subresources, command_normalization_barriers,
              r_command->normalization_barrier_index, r_command->normalization_barrier_count);
        }

        current_tracker->in_parent_dirty_list = false;
        current_tracker = current_tracker->next_shared;
      }

      resource_tracker->dirty_shared_list = nullptr;
    }

    // 上面主要处理了dirty list

    // 如果只是一个切片就用父亲的
    bool resource_has_parent = resource_tracker->parent != nullptr;
    ResourceTracker* search_tracker =
        resource_has_parent ? resource_tracker->parent : resource_tracker;

    bool diff_usage = resource_tracker->usage != new_resource_usage;
    bool diff_layout = _usage_to_image_layout(resource_tracker->usage) !=
                       _usage_to_image_layout(new_resource_usage);
    bool write_usage_after_write = (write_usage && search_tracker->write_cmd_idx >= 0);
    // 如果需要transition 以及 write后write 添加barrier
    if (diff_layout || write_usage_after_write) {
      // A barrier must be pushed if the usage is different of it's a write usage and there was already a command that wrote to this resource previously.
      // 在之前cmd完成之前等待
      if (resource_tracker->is_texture()) {
        if (resource_tracker->usage_access.is_empty()) {
          // FIXME: If the tracker does not know the previous type of usage, assume the generic memory write one.
          // Tracking access bits across texture slices can be tricky, so this failsafe can be removed once that's improved.
          resource_tracker->usage_access =
              RDD::BARRIER_ACCESS_MEMORY_WRITE_BIT;  // 假定之前是写，现在又写
        }
        _add_texture_barrier_to_command(
            resource_tracker->texture_driver_id, resource_tracker->usage_access, new_usage_access,
            resource_tracker->usage, new_resource_usage, resource_tracker->texture_subresources,
            command_transition_barriers, r_command->transition_barrier_index,
            r_command->transition_barrier_count);
      } else {  // is buffer
#if USE_BUFFER_BARRIERS
        _add_buffer_barrier_to_command(
            resource_tracker->buffer_driver_id, resource_tracker->usage_access, new_usage_access,
            resource_tracker->buffer_range, r_command->buffer_barrier_index,
            r_command->buffer_barrier_count);
#endif
        // FIXME: Memory barriers are currently pushed regardless of whether buffer barriers are being used or not. Refer to the comment on the
        // definition of USE_BUFFER_BARRIERS for the reason behind this. This can be fixed to be one case or the other once it's been confirmed
        // the buffer and memory barrier behavior discrepancy has been solved.

        r_command->memory_barrier.src_access = resource_tracker->usage_access;
        r_command->memory_barrier.dst_access = new_usage_access;
      }
    }  // if (diff_usage || write_usage_after_write)

    // Always update the access of the tracker according to the latest usage.
    resource_tracker->usage_access = new_usage_access;
    // transition and it should therefore be considered a write.
    if (diff_layout) {
      write_usage = true;
      resource_tracker->usage = new_resource_usage;
    }
    if (search_tracker->has_write() && search_tracker->write_cmd_list_enable) {
      // Make this command adjacent to any commands that wrote to this resource and intersect with the slice if it applies.
      // For buffers or textures that never use slices, this list will only be one element long at most.
      int32_t write_cmd_idx = search_tracker->write_cmd_idx;  // 能保证write_cmd_idx都是属于该资源的
      int32_t previous_write_cmd_idx = -1;
      while (write_cmd_idx >= 0) {
        // 查找所有的write，如果有交集就添加依赖
        const RecordedSliceListNode& write_list_node = write_slice_list_nodes[write_cmd_idx];
        // 如果直接对father资源写，或者对切片写但是有交集
        if (!resource_has_parent || search_tracker_rect.intersects(write_list_node.subresources)) {
          if (write_list_node.command_index == p_command_index) {
            ERR_FAIL_COND_MSG(!resource_has_parent, "Command can't have itself as a dependency.");
          } else {
            // Command is dependent on this command. Add this command to the adjacency list of the write command.
            // 本写指令依赖于该命令。
            _add_adjacent_command(write_list_node.command_index, p_command_index, r_command);

            if (resource_has_parent && write_usage &&
                search_tracker_rect.encloses(write_list_node.subresources)) {
              // Eliminate redundant writes from the list.
              // 如果要写切片，并且写的范围比该命令的范围大，就不需要之前的写了，以后查阅这个写
              // 写命令仍然存在

              if (previous_write_cmd_idx >= 0) {
                RecordedSliceListNode& previous_list_node =
                    write_slice_list_nodes[previous_write_cmd_idx];
                previous_list_node.next_list_index = write_list_node.next_list_index;
              } else {
                search_tracker->write_cmd_idx = write_list_node.next_list_index;
              }

              write_cmd_idx = write_list_node.next_list_index;
              continue;
            }
          }
        }

        previous_write_cmd_idx = write_cmd_idx;
        write_cmd_idx = write_list_node.next_list_index;
      }
    } else if (search_tracker->has_write() && !search_tracker->write_cmd_list_enable) {
      // The index is just the latest command index that wrote to the resource.
      if (search_tracker->write_cmd_idx == p_command_index) {
        ERR_FAIL_MSG("Command can't have itself as a dependency.");
      } else {
        _add_adjacent_command(search_tracker->write_cmd_idx, p_command_index, r_command);
      }
    }  // else : 该资源没有写
    // update write_cmd_list
    if (write_usage) {
      // 由此可见父的write包含了所有对切片的写
      if (resource_has_parent) {
        if (!search_tracker->write_cmd_list_enable && search_tracker->write_cmd_idx >= 0) {
          // Write command list was not being used but there was a write command recorded. Add a new node with the entire parent resource's subresources and the recorded command index to the list.
          search_tracker->write_cmd_idx = _add_to_write_list(
              search_tracker->write_cmd_idx, search_tracker->get_subresources(), -1);  // head
        }

        search_tracker->write_cmd_idx =
            _add_to_write_list(p_command_index, search_tracker_rect, search_tracker->write_cmd_idx);
        search_tracker->write_cmd_list_enable = true;
      } else {  // 如果不是切片，直接添加
        search_tracker->write_cmd_idx = p_command_index;
        search_tracker->write_cmd_list_enable = false;
      }

      // We add this command to the adjacency list of all commands that were reading from the entire resource.
      int32_t read_full_cmd_idx = search_tracker->read_full_cmd_idx;
      while (read_full_cmd_idx >= 0) {
        int32_t read_full_command_index = command_list_nodes[read_full_cmd_idx].command_index;
        int32_t read_full_next_index = command_list_nodes[read_full_cmd_idx].next_list_index;
        if (read_full_command_index == p_command_index) {
          if (!resource_has_parent) {
            // Only slices are allowed to be in different usages in the same command as they are guaranteed to have no overlap in the same command.
            ERR_FAIL_MSG("Command can't have itself as a dependency.");
          }
        } else {
          // Add this command to the adjacency list of each command that was reading this resource.
          _add_adjacent_command(read_full_command_index, p_command_index, r_command);
        }

        read_full_cmd_idx = read_full_next_index;
      }
      if (!resource_has_parent) {
        // Clear the full list if this resource is not a slice.
        search_tracker->read_full_cmd_idx = -1;
      }
      // We add this command to the adjacency list of all commands that were reading from resource slices.
      int32_t previous_slice_command_list_index = -1;
      int32_t read_slice_command_list_index = search_tracker->read_slice_cmd_idx;
      while (read_slice_command_list_index >= 0) {
        const RecordedSliceListNode& read_list_node =
            read_slice_list_nodes[read_slice_command_list_index];
        // 如果不是切片或者切片且完全包含该命令(后面会加入这个作为新的)
        if (!resource_has_parent || search_tracker_rect.encloses(read_list_node.subresources)) {
          if (previous_slice_command_list_index >= 0) {
            // Erase this element and connect the previous one to the next element.
            read_slice_list_nodes[previous_slice_command_list_index].next_list_index =
                read_list_node.next_list_index;
          } else {
            // Erase this element from the head of the list.
            DEV_ASSERT(search_tracker->read_slice_cmd_idx == read_slice_command_list_index);
            search_tracker->read_slice_cmd_idx = read_list_node.next_list_index;
          }

          // Advance to the next element.
          read_slice_command_list_index = read_list_node.next_list_index;
        } else {
          previous_slice_command_list_index = read_slice_command_list_index;
          read_slice_command_list_index = read_list_node.next_list_index;
        }
        if (!resource_has_parent || search_tracker_rect.intersects(read_list_node.subresources)) {
          // Add this command to the adjacency list of each command that was reading this resource.
          // We only add the dependency if there's an intersection between slices or this resource isn't a slice.
          _add_adjacent_command(read_list_node.command_index, p_command_index, r_command);
        }
      }
    }  // if (write_usage)
    // read_usage
    else if (resource_has_parent) {
      // We add a read dependency to the tracker to indicate this command reads from the resource slice.
      search_tracker->read_slice_cmd_idx = _add_to_slice_read_list(
          p_command_index, resource_tracker_rect, search_tracker->read_slice_cmd_idx);
    } else {
      // We add a read dependency to the tracker to indicate this command reads from the entire resource.
      search_tracker->read_full_cmd_idx =
          _add_to_command_list(p_command_index, search_tracker->read_slice_cmd_idx);
    }

  }  // for each resource tracker
}

int32_t RenderingDeviceGraph::_add_to_slice_read_list(int32_t p_command_index,
                                                      Rect2i p_subresources, int32_t p_list_index) {
  DEV_ASSERT(p_command_index < int32_t(command_count));
  DEV_ASSERT(p_list_index < int32_t(read_slice_list_nodes.size()));

  int32_t next_index = int32_t(read_slice_list_nodes.size());
  read_slice_list_nodes.resize(next_index + 1);

  RecordedSliceListNode& new_node = read_slice_list_nodes[next_index];
  new_node.command_index = p_command_index;
  new_node.next_list_index = p_list_index;
  new_node.subresources = p_subresources;
  return next_index;
}
int32_t RenderingDeviceGraph::_add_to_write_list(int32_t p_command_index, Rect2i p_subresources,
                                                 int32_t p_list_index) {
  DEV_ASSERT(p_command_index < int32_t(command_count));
  DEV_ASSERT(p_list_index < int32_t(write_slice_list_nodes.size()));

  int32_t next_index = int32_t(write_slice_list_nodes.size());
  write_slice_list_nodes.resize(next_index + 1);

  RecordedSliceListNode& new_node = write_slice_list_nodes[next_index];
  new_node.command_index = p_command_index;
  new_node.next_list_index = p_list_index;
  new_node.subresources = p_subresources;
  return next_index;
}

#if USE_BUFFER_BARRIERS
void RenderingDeviceGraph::_add_buffer_barrier_to_command(
    RDD::BufferID p_buffer_id, BitField<RDD::BarrierAccessBits> p_src_access,
    BitField<RDD::BarrierAccessBits> p_dst_access, RDD::BufferRange p_range,
    int32_t& r_barrier_index, int32_t& r_barrier_count) {
  if (!driver_honors_barriers) {
    return;
  }

  if (r_barrier_index < 0) {
    r_barrier_index = command_buffer_barriers.size();
  }

  RDD::BufferBarrier buffer_barrier;
  buffer_barrier.buffer = p_buffer_id;
  buffer_barrier.src_access = p_src_access;
  buffer_barrier.dst_access = p_dst_access;
  buffer_barrier.range = p_range;
  command_buffer_barriers.push_back(buffer_barrier);
  r_barrier_count++;
}
#endif
void RenderingDeviceGraph::_add_texture_barrier_to_command(
    RDD::TextureID p_texture_id, BitField<RDD::BarrierAccessBits> p_src_access,
    BitField<RDD::BarrierAccessBits> p_dst_access, ResourceUsage p_prev_usage,
    ResourceUsage p_next_usage, RDD::TextureSubresourceRange p_subresources,
    LocalVector<RDD::TextureBarrier>& r_barrier_vector, int32_t& r_barrier_index,
    int32_t& r_barrier_count) {
  if (!driver_honors_barriers) {
    return;
  }

  if (r_barrier_index < 0) {
    r_barrier_index = r_barrier_vector.size();
  }

  RDD::TextureBarrier texture_barrier;
  texture_barrier.texture = p_texture_id;
  texture_barrier.src_access = p_src_access;  // @info: 0 时为布局转换
  texture_barrier.dst_access = p_dst_access;
  texture_barrier.prev_layout = _usage_to_image_layout(p_prev_usage);
  texture_barrier.next_layout = _usage_to_image_layout(p_next_usage);
  texture_barrier.subresources = p_subresources;
  r_barrier_vector.push_back(texture_barrier);
  r_barrier_count++;
}

void RenderingDeviceGraph::_add_adjacent_command(int32_t p_previous_command_index,
                                                 int32_t p_command_index,
                                                 RecordedCommand* r_command) {
  const uint32_t previous_command_data_offset = command_data_offsets[p_previous_command_index];
  RecordedCommand& previous_command =
      *reinterpret_cast<RecordedCommand*>(&command_data[previous_command_data_offset]);
  previous_command.adjacent_command_list_index = _add_to_command_list(
      p_command_index, previous_command.adjacent_command_list_index);  // 倒着插入
  previous_command.next_stages = previous_command.next_stages | r_command->self_stages;
  r_command->previous_stages = r_command->previous_stages | previous_command.self_stages;
}

int32_t RenderingDeviceGraph::_add_to_command_list(int32_t p_command_index, int32_t p_list_index) {
  DEV_ASSERT(p_command_index < int32_t(command_count));
  DEV_ASSERT(p_list_index < int32_t(command_list_nodes.size()));

  int32_t next_index = int32_t(command_list_nodes.size());
  command_list_nodes.resize(next_index + 1);

  RecordedCommandListNode& new_node = command_list_nodes[next_index];
  new_node.command_index = p_command_index;
  new_node.next_list_index = p_list_index;
  return next_index;
}

// allocate command
RenderingDeviceGraph::DrawListInstruction* RenderingDeviceGraph::_allocate_draw_list_instruction(
    uint32_t p_instruction_size) {
  uint32_t draw_list_data_offset = draw_instruction_list.data.size();
  draw_instruction_list.data.resize(draw_list_data_offset + p_instruction_size);
  return reinterpret_cast<DrawListInstruction*>(&draw_instruction_list.data[draw_list_data_offset]);
}

void RenderingDeviceGraph::initialize(RDD* p_driver, RenderingContextDriver::Device p_device,
                                      uint32_t p_frame_count,
                                      RDD::CommandQueueFamilyID p_secondary_command_queue_family,
                                      uint32_t p_secondary_command_buffers_per_frame) {
  driver = p_driver;
  device = p_device;
  frames.resize(p_frame_count);

  for (uint32_t i = 0; i < p_frame_count; i++) {
    frames[i].secondary_command_buffers.resize(p_secondary_command_buffers_per_frame);

    for (uint32_t j = 0; j < p_secondary_command_buffers_per_frame; j++) {
      SecondaryCommandBuffer& secondary = frames[i].secondary_command_buffers[j];
      secondary.command_pool = driver->command_pool_create(p_secondary_command_queue_family,
                                                           RDD::COMMAND_BUFFER_TYPE_SECONDARY);
      secondary.command_buffer = driver->command_buffer_create(secondary.command_pool);
      secondary.task = WorkerThreadPool::INVALID_TASK_ID;
    }
  }

  driver_honors_barriers = driver->api_trait_get(RDD::API_TRAIT_HONORS_PIPELINE_BARRIERS);
  driver_clears_with_copy_engine = driver->api_trait_get(RDD::API_TRAIT_CLEARS_WITH_COPY_ENGINE);
}

RenderingDeviceGraph::RecordedCommand* RenderingDeviceGraph::_allocate_command(
    uint32_t p_command_size, int32_t& r_command_index) {
  uint32_t command_data_offset = command_data.size();
  command_data_offsets.push_back(command_data_offset);
  command_data.resize(command_data_offset + p_command_size);
  r_command_index = command_count++;  // 额外需要增加这个
  RecordedCommand* new_command =
      reinterpret_cast<RecordedCommand*>(&command_data[command_data_offset]);
  *new_command = RecordedCommand();  // 需要初始化
  return new_command;
}

RenderingDeviceGraph::ComputeListInstruction*
RenderingDeviceGraph::_allocate_compute_list_instruction(uint32_t p_instruction_size) {
  uint32_t compute_list_data_offset = compute_instruction_list.data.size();
  compute_instruction_list.data.resize(compute_list_data_offset + p_instruction_size);
  return reinterpret_cast<ComputeListInstruction*>(
      &compute_instruction_list.data[compute_list_data_offset]);
}

void RenderingDeviceGraph::add_buffer_clear(RDD::BufferID p_dst, ResourceTracker* p_dst_tracker,
                                            uint32_t p_offset, uint32_t p_size) {
  DEV_ASSERT(p_dst_tracker != nullptr);

  int32_t command_index;
  RecordedBufferClearCommand* command = static_cast<RecordedBufferClearCommand*>(
      _allocate_command(sizeof(RecordedBufferClearCommand), command_index));
  command->type = RecordedCommand::TYPE_BUFFER_CLEAR;
  command->self_stages = RDD::PIPELINE_STAGE_COPY_BIT;
  command->buffer = p_dst;
  command->offset = p_offset;
  command->size = p_size;

  ResourceUsage usage = RESOURCE_USAGE_COPY_TO;
  _add_command_to_graph(&p_dst_tracker, &usage, 1, command_index, command);
}

void RenderingDeviceGraph::add_buffer_copy(RDD::BufferID p_src, ResourceTracker* p_src_tracker,
                                           RDD::BufferID p_dst, ResourceTracker* p_dst_tracker,
                                           RDD::BufferCopyRegion p_region) {
  // Source tracker is allowed to be null as it could be a read-only buffer.
  DEV_ASSERT(p_dst_tracker != nullptr);

  int32_t command_index;
  RecordedBufferCopyCommand* command = static_cast<RecordedBufferCopyCommand*>(
      _allocate_command(sizeof(RecordedBufferCopyCommand), command_index));
  command->type = RecordedCommand::TYPE_BUFFER_COPY;
  command->self_stages = RDD::PIPELINE_STAGE_COPY_BIT;
  command->source = p_src;
  command->destination = p_dst;
  command->region = p_region;

  ResourceTracker* trackers[2] = {p_dst_tracker, p_src_tracker};
  ResourceUsage usages[2] = {RESOURCE_USAGE_COPY_TO, RESOURCE_USAGE_COPY_FROM};
  _add_command_to_graph(trackers, usages, p_src_tracker != nullptr ? 2 : 1, command_index, command);
}

void RenderingDeviceGraph::add_buffer_get_data(RDD::BufferID p_src, ResourceTracker* p_src_tracker,
                                               RDD::BufferID p_dst,
                                               RDD::BufferCopyRegion p_region) {
  // Source tracker is allowed to be null as it could be a read-only buffer.
  int32_t command_index;
  RecordedBufferGetDataCommand* command = static_cast<RecordedBufferGetDataCommand*>(
      _allocate_command(sizeof(RecordedBufferGetDataCommand), command_index));
  command->type = RecordedCommand::TYPE_BUFFER_GET_DATA;
  command->self_stages = RDD::PIPELINE_STAGE_COPY_BIT;
  command->source = p_src;
  command->destination = p_dst;
  command->region = p_region;

  if (p_src_tracker != nullptr) {
    ResourceUsage usage = RESOURCE_USAGE_COPY_FROM;
    _add_command_to_graph(&p_src_tracker, &usage, 1, command_index, command);
  } else {
    _add_command_to_graph(nullptr, nullptr, 0, command_index, command);
  }
}

// @? buffer_update是复制pData到buffer。这里是复制若干个buffer到该buffer
void RenderingDeviceGraph::add_buffer_update(RDD::BufferID p_dst, ResourceTracker* p_dst_tracker,
                                             VectorView<RecordedBufferCopy> p_buffer_copies) {
  DEV_ASSERT(p_dst_tracker != nullptr);

  size_t buffer_copies_size = p_buffer_copies.size() * sizeof(RecordedBufferCopy);
  uint64_t command_size = sizeof(RecordedBufferUpdateCommand) + buffer_copies_size;
  int32_t command_index;
  RecordedBufferUpdateCommand* command =
      static_cast<RecordedBufferUpdateCommand*>(_allocate_command(command_size, command_index));
  command->type = RecordedCommand::TYPE_BUFFER_UPDATE;
  command->self_stages = RDD::PIPELINE_STAGE_COPY_BIT;
  command->destination = p_dst;
  command->buffer_copies_count = p_buffer_copies.size();

  RecordedBufferCopy* buffer_copies = command->buffer_copies();
  for (uint32_t i = 0; i < command->buffer_copies_count; i++) {
    buffer_copies[i] = p_buffer_copies[i];
  }

  ResourceUsage buffer_usage = RESOURCE_USAGE_COPY_TO;
  _add_command_to_graph(&p_dst_tracker, &buffer_usage, 1, command_index, command);
}

void RenderingDeviceGraph::add_compute_list_begin() {
  compute_instruction_list.clear();
  compute_instruction_list.index++;
}

void RenderingDeviceGraph::add_compute_list_bind_pipeline(RDD::PipelineID p_pipeline) {
  ComputeListBindPipelineInstruction* instruction =
      reinterpret_cast<ComputeListBindPipelineInstruction*>(
          _allocate_compute_list_instruction(sizeof(ComputeListBindPipelineInstruction)));
  instruction->type = ComputeListInstruction::TYPE_BIND_PIPELINE;
  instruction->pipeline = p_pipeline;
  compute_instruction_list.stages.set_flag(RDD::PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}

void RenderingDeviceGraph::add_compute_list_bind_uniform_set(RDD::ShaderID p_shader,
                                                             RDD::UniformSetID p_uniform_set,
                                                             uint32_t set_index) {
  ComputeListBindUniformSetInstruction* instruction =
      reinterpret_cast<ComputeListBindUniformSetInstruction*>(
          _allocate_compute_list_instruction(sizeof(ComputeListBindUniformSetInstruction)));
  instruction->type = ComputeListInstruction::TYPE_BIND_UNIFORM_SET;
  instruction->shader = p_shader;
  instruction->uniform_set = p_uniform_set;
  instruction->set_index = set_index;
}

void RenderingDeviceGraph::add_compute_list_dispatch(uint32_t p_x_groups, uint32_t p_y_groups,
                                                     uint32_t p_z_groups) {
  ComputeListDispatchInstruction* instruction = reinterpret_cast<ComputeListDispatchInstruction*>(
      _allocate_compute_list_instruction(sizeof(ComputeListDispatchInstruction)));
  instruction->type = ComputeListInstruction::TYPE_DISPATCH;
  instruction->x_groups = p_x_groups;
  instruction->y_groups = p_y_groups;
  instruction->z_groups = p_z_groups;
}

void RenderingDeviceGraph::add_compute_list_dispatch_indirect(RDD::BufferID p_buffer,
                                                              uint32_t p_offset) {
  ComputeListDispatchIndirectInstruction* instruction =
      reinterpret_cast<ComputeListDispatchIndirectInstruction*>(
          _allocate_compute_list_instruction(sizeof(ComputeListDispatchIndirectInstruction)));
  instruction->type = ComputeListInstruction::TYPE_DISPATCH_INDIRECT;
  instruction->buffer = p_buffer;
  instruction->offset = p_offset;
  compute_instruction_list.stages.set_flag(RDD::PIPELINE_STAGE_DRAW_INDIRECT_BIT);
}

void RenderingDeviceGraph::add_compute_list_set_push_constant(RDD::ShaderID p_shader,
                                                              const void* p_data,
                                                              uint32_t p_data_size) {
  uint32_t instruction_size = sizeof(ComputeListSetPushConstantInstruction) + p_data_size;
  ComputeListSetPushConstantInstruction* instruction =
      reinterpret_cast<ComputeListSetPushConstantInstruction*>(
          _allocate_compute_list_instruction(instruction_size));
  instruction->type = ComputeListInstruction::TYPE_SET_PUSH_CONSTANT;
  instruction->size = p_data_size;
  instruction->shader = p_shader;
  memcpy(instruction->data(), p_data, p_data_size);
}

void RenderingDeviceGraph::add_compute_list_uniform_set_prepare_for_use(
    RDD::ShaderID p_shader, RDD::UniformSetID p_uniform_set, uint32_t set_index) {
  ComputeListUniformSetPrepareForUseInstruction* instruction =
      reinterpret_cast<ComputeListUniformSetPrepareForUseInstruction*>(
          _allocate_compute_list_instruction(
              sizeof(ComputeListUniformSetPrepareForUseInstruction)));
  instruction->type = ComputeListInstruction::TYPE_UNIFORM_SET_PREPARE_FOR_USE;
  instruction->shader = p_shader;
  instruction->uniform_set = p_uniform_set;
  instruction->set_index = set_index;
}

void RenderingDeviceGraph::add_compute_list_usage(ResourceTracker* p_tracker,
                                                  ResourceUsage p_usage) {
  DEV_ASSERT(p_tracker != nullptr);

  p_tracker->reset_if_outdated(tracking_frame);

  if (p_tracker->compute_idx != compute_instruction_list.index) {
    compute_instruction_list.command_trackers.push_back(p_tracker);
    compute_instruction_list.command_tracker_usages.push_back(p_usage);
    p_tracker->compute_idx = compute_instruction_list.index;
    p_tracker->compute_usage = p_usage;
  }
#ifdef DEV_ENABLED
  else if (p_tracker->compute_usage != p_usage) {
    ERR_FAIL_MSG(
        vformat("Tracker can't have more thasage in the same compute list. Compute "
                "list usage is %d and the requested usage is %d.",
                p_tracker->compute_usage, p_usage));
  }
#endif
}

void RenderingDeviceGraph::add_compute_list_usages(VectorView<ResourceTracker*> p_trackers,
                                                   VectorView<ResourceUsage> p_usages) {
  DEV_ASSERT(p_trackers.size() == p_usages.size());

  for (uint32_t i = 0; i < p_trackers.size(); i++) {
    add_compute_list_usage(p_trackers[i], p_usages[i]);
  }
}
// 把所有compute commands塞到一起
// 没有多线程的部分？
void RenderingDeviceGraph::add_compute_list_end() {
  int32_t command_index;
  uint32_t instruction_data_size = compute_instruction_list.data.size();
  uint32_t command_size = sizeof(RecordedComputeListCommand) + instruction_data_size;
  RecordedComputeListCommand* command =
      static_cast<RecordedComputeListCommand*>(_allocate_command(command_size, command_index));
  command->type = RecordedCommand::TYPE_COMPUTE_LIST;
  command->self_stages = compute_instruction_list.stages;
  command->instruction_data_size = instruction_data_size;
  memcpy(command->instruction_data(), compute_instruction_list.data.ptr(), instruction_data_size);
  _add_command_to_graph(compute_instruction_list.command_trackers.ptr(),
                        compute_instruction_list.command_tracker_usages.ptr(),
                        compute_instruction_list.command_trackers.size(), command_index, command);
}
///
///
/// *** DRAW LIST ***
///
///

void RenderingDeviceGraph::add_draw_list_begin(RDD::RenderPassID p_render_pass,
                                               RDD::FramebufferID p_framebuffer, Rect2i p_region,
                                               VectorView<RDD::RenderPassClearValue> p_clear_values,
                                               bool p_uses_color, bool p_uses_depth) {
  draw_instruction_list.clear();
  draw_instruction_list.index++;
  draw_instruction_list.render_pass = p_render_pass;
  draw_instruction_list.framebuffer = p_framebuffer;
  draw_instruction_list.region = p_region;
  draw_instruction_list.clear_values.resize(p_clear_values.size());
  for (uint32_t i = 0; i < p_clear_values.size(); i++) {
    draw_instruction_list.clear_values[i] = p_clear_values[i];
  }

  if (p_uses_color) {
    draw_instruction_list.stages.set_flag(RDD::PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
  }

  if (p_uses_depth) {
    draw_instruction_list.stages.set_flag(RDD::PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
    draw_instruction_list.stages.set_flag(RDD::PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
  }
}

void RenderingDeviceGraph::add_draw_list_bind_index_buffer(RDD::BufferID p_buffer,
                                                           RDD::IndexBufferFormat p_format,
                                                           uint32_t p_offset) {
  DrawListBindIndexBufferInstruction* instruction =
      reinterpret_cast<DrawListBindIndexBufferInstruction*>(
          _allocate_draw_list_instruction(sizeof(DrawListBindIndexBufferInstruction)));
  instruction->type = DrawListInstruction::TYPE_BIND_INDEX_BUFFER;
  instruction->buffer = p_buffer;
  instruction->format = p_format;
  instruction->offset = p_offset;

  if (instruction->buffer.id != 0) {
    draw_instruction_list.stages.set_flag(RDD::PIPELINE_STAGE_VERTEX_INPUT_BIT);
  }
}

void RenderingDeviceGraph::add_draw_list_bind_pipeline(
    RDD::PipelineID p_pipeline, BitField<RDD::PipelineStageBits> p_pipeline_stage_bits) {
  DrawListBindPipelineInstruction* instruction = reinterpret_cast<DrawListBindPipelineInstruction*>(
      _allocate_draw_list_instruction(sizeof(DrawListBindPipelineInstruction)));
  instruction->type = DrawListInstruction::TYPE_BIND_PIPELINE;
  instruction->pipeline = p_pipeline;
  draw_instruction_list.stages = draw_instruction_list.stages | p_pipeline_stage_bits;
}

void RenderingDeviceGraph::add_draw_list_bind_uniform_set(RDD::ShaderID p_shader,
                                                          RDD::UniformSetID p_uniform_set,
                                                          uint32_t set_index) {
  DrawListBindUniformSetInstruction* instruction =
      reinterpret_cast<DrawListBindUniformSetInstruction*>(
          _allocate_draw_list_instruction(sizeof(DrawListBindUniformSetInstruction)));
  instruction->type = DrawListInstruction::TYPE_BIND_UNIFORM_SET;
  instruction->shader = p_shader;
  instruction->uniform_set = p_uniform_set;
  instruction->set_index = set_index;
}

void RenderingDeviceGraph::add_draw_list_bind_vertex_buffers(
    VectorView<RDD::BufferID> p_vertex_buffers, VectorView<uint64_t> p_vertex_buffer_offsets) {
  DEV_ASSERT(p_vertex_buffers.size() == p_vertex_buffer_offsets.size());

  uint32_t instruction_size = sizeof(DrawListBindVertexBuffersInstruction) +
                              sizeof(RDD::BufferID) * p_vertex_buffers.size() +
                              sizeof(uint64_t) * p_vertex_buffer_offsets.size();
  DrawListBindVertexBuffersInstruction* instruction =
      reinterpret_cast<DrawListBindVertexBuffersInstruction*>(
          _allocate_draw_list_instruction(instruction_size));
  instruction->type = DrawListInstruction::TYPE_BIND_VERTEX_BUFFERS;
  instruction->vertex_buffers_count = p_vertex_buffers.size();

  RDD::BufferID* vertex_buffers = instruction->vertex_buffers();
  uint64_t* vertex_buffer_offsets = instruction->vertex_buffer_offsets();
  for (uint32_t i = 0; i < instruction->vertex_buffers_count; i++) {
    vertex_buffers[i] = p_vertex_buffers[i];
    vertex_buffer_offsets[i] = p_vertex_buffer_offsets[i];
  }

  if (instruction->vertex_buffers_count > 0) {
    draw_instruction_list.stages.set_flag(RDD::PIPELINE_STAGE_VERTEX_INPUT_BIT);
  }
}

void RenderingDeviceGraph::add_draw_list_clear_attachments(
    VectorView<RDD::AttachmentClear> p_attachments_clear,
    VectorView<Rect2i> p_attachments_clear_rect) {
  uint32_t instruction_size = sizeof(DrawListClearAttachmentsInstruction) +
                              sizeof(RDD::AttachmentClear) * p_attachments_clear.size() +
                              sizeof(Rect2i) * p_attachments_clear_rect.size();
  DrawListClearAttachmentsInstruction* instruction =
      reinterpret_cast<DrawListClearAttachmentsInstruction*>(
          _allocate_draw_list_instruction(instruction_size));
  instruction->type = DrawListInstruction::TYPE_CLEAR_ATTACHMENTS;
  instruction->attachments_clear_count = p_attachments_clear.size();
  instruction->attachments_clear_rect_count = p_attachments_clear_rect.size();

  RDD::AttachmentClear* attachments_clear = instruction->attachments_clear();
  Rect2i* attachments_clear_rect = instruction->attachments_clear_rect();
  for (uint32_t i = 0; i < instruction->attachments_clear_count; i++) {
    attachments_clear[i] = p_attachments_clear[i];
  }

  for (uint32_t i = 0; i < instruction->attachments_clear_rect_count; i++) {
    attachments_clear_rect[i] = p_attachments_clear_rect[i];
  }
}

void RenderingDeviceGraph::add_draw_list_draw(uint32_t p_vertex_count, uint32_t p_instance_count) {
  DrawListDrawInstruction* instruction = reinterpret_cast<DrawListDrawInstruction*>(
      _allocate_draw_list_instruction(sizeof(DrawListDrawInstruction)));
  instruction->type = DrawListInstruction::TYPE_DRAW;
  instruction->vertex_count = p_vertex_count;
  instruction->instance_count = p_instance_count;
}

void RenderingDeviceGraph::add_draw_list_draw_indexed(uint32_t p_index_count,
                                                      uint32_t p_instance_count,
                                                      uint32_t p_first_index) {
  DrawListDrawIndexedInstruction* instruction = reinterpret_cast<DrawListDrawIndexedInstruction*>(
      _allocate_draw_list_instruction(sizeof(DrawListDrawIndexedInstruction)));
  instruction->type = DrawListInstruction::TYPE_DRAW_INDEXED;
  instruction->index_count = p_index_count;
  instruction->instance_count = p_instance_count;
  instruction->first_index = p_first_index;
}

void RenderingDeviceGraph::add_draw_list_execute_commands(RDD::CommandBufferID p_command_buffer) {
  DrawListExecuteCommandsInstruction* instruction =
      reinterpret_cast<DrawListExecuteCommandsInstruction*>(
          _allocate_draw_list_instruction(sizeof(DrawListExecuteCommandsInstruction)));
  instruction->type = DrawListInstruction::TYPE_EXECUTE_COMMANDS;
  instruction->command_buffer = p_command_buffer;
}

void RenderingDeviceGraph::add_draw_list_next_subpass(
    RDD::CommandBufferType p_command_buffer_type) {
  DrawListNextSubpassInstruction* instruction = reinterpret_cast<DrawListNextSubpassInstruction*>(
      _allocate_draw_list_instruction(sizeof(DrawListNextSubpassInstruction)));
  instruction->type = DrawListInstruction::TYPE_NEXT_SUBPASS;
  instruction->command_buffer_type = p_command_buffer_type;
}

void RenderingDeviceGraph::add_draw_list_set_blend_constants(const Color& p_color) {
  DrawListSetBlendConstantsInstruction* instruction =
      reinterpret_cast<DrawListSetBlendConstantsInstruction*>(
          _allocate_draw_list_instruction(sizeof(DrawListSetBlendConstantsInstruction)));
  instruction->type = DrawListInstruction::TYPE_SET_BLEND_CONSTANTS;
  instruction->color = p_color;
}

void RenderingDeviceGraph::add_draw_list_set_line_width(float p_width) {
  DrawListSetLineWidthInstruction* instruction = reinterpret_cast<DrawListSetLineWidthInstruction*>(
      _allocate_draw_list_instruction(sizeof(DrawListSetLineWidthInstruction)));
  instruction->type = DrawListInstruction::TYPE_SET_LINE_WIDTH;
  instruction->width = p_width;
}

void RenderingDeviceGraph::add_draw_list_set_push_constant(RDD::ShaderID p_shader,
                                                           const void* p_data,
                                                           uint32_t p_data_size) {
  uint32_t instruction_size = sizeof(DrawListSetPushConstantInstruction) + p_data_size;
  DrawListSetPushConstantInstruction* instruction =
      reinterpret_cast<DrawListSetPushConstantInstruction*>(
          _allocate_draw_list_instruction(instruction_size));
  instruction->type = DrawListInstruction::TYPE_SET_PUSH_CONSTANT;
  instruction->size = p_data_size;
  instruction->shader = p_shader;
  memcpy(instruction->data(), p_data, p_data_size);
}

void RenderingDeviceGraph::add_draw_list_set_scissor(Rect2i p_rect) {
  DrawListSetScissorInstruction* instruction = reinterpret_cast<DrawListSetScissorInstruction*>(
      _allocate_draw_list_instruction(sizeof(DrawListSetScissorInstruction)));
  instruction->type = DrawListInstruction::TYPE_SET_SCISSOR;
  instruction->rect = p_rect;
}

void RenderingDeviceGraph::add_draw_list_set_viewport(Rect2i p_rect) {
  DrawListSetViewportInstruction* instruction = reinterpret_cast<DrawListSetViewportInstruction*>(
      _allocate_draw_list_instruction(sizeof(DrawListSetViewportInstruction)));
  instruction->type = DrawListInstruction::TYPE_SET_VIEWPORT;
  instruction->rect = p_rect;
}

void RenderingDeviceGraph::add_draw_list_uniform_set_prepare_for_use(
    RDD::ShaderID p_shader, RDD::UniformSetID p_uniform_set, uint32_t set_index) {
  DrawListUniformSetPrepareForUseInstruction* instruction =
      reinterpret_cast<DrawListUniformSetPrepareForUseInstruction*>(
          _allocate_draw_list_instruction(sizeof(DrawListUniformSetPrepareForUseInstruction)));
  instruction->type = DrawListInstruction::TYPE_UNIFORM_SET_PREPARE_FOR_USE;
  instruction->shader = p_shader;
  instruction->uniform_set = p_uniform_set;
  instruction->set_index = set_index;
}

void RenderingDeviceGraph::add_draw_list_usage(ResourceTracker* p_tracker, ResourceUsage p_usage) {
  p_tracker->reset_if_outdated(tracking_frame);

  if (p_tracker->draw_idx != draw_instruction_list.index) {
    draw_instruction_list.command_trackers.push_back(p_tracker);
    draw_instruction_list.command_tracker_usages.push_back(p_usage);
    p_tracker->draw_idx = draw_instruction_list.index;
    p_tracker->draw_usage = p_usage;
  }
#ifdef DEV_ENABLED
  else if (p_tracker->draw_usage != p_usage) {
    ERR_FAIL_MSG(
        vformat("Tracker can't have more than one type of usage in the same draw list. Draw list "
                "usage is %d and the requested usage is %d.",
                p_tracker->draw_usage, p_usage));
  }
#endif
}
// uniform_set 的绑定 需要调用
void RenderingDeviceGraph::add_draw_list_usages(VectorView<ResourceTracker*> p_trackers,
                                                VectorView<ResourceUsage> p_usages) {
  DEV_ASSERT(p_trackers.size() == p_usages.size());

  for (uint32_t i = 0; i < p_trackers.size(); i++) {
    add_draw_list_usage(p_trackers[i], p_usages[i]);
  }
}

void RenderingDeviceGraph::add_draw_list_end() {
  //Arbitrary size threshold to evaluate if it'd be best to record the draw list on the background as a secondary buffer.
  const uint32_t instruction_data_threshold_for_secondary = 1000;
  RDD::CommandBufferType command_buffer_type;
  uint32_t& secondary_buffers_used = frames[frame].secondary_command_buffers_used;
  // 如果大于阈值，就用secondary command buffer，否则用primary
  if (draw_instruction_list.data.size() > instruction_data_threshold_for_secondary &&
      secondary_buffers_used < frames[frame].secondary_command_buffers.size()) {
    // Copy the current instruction list data into another array that will be used by the secondary command buffer worker.
    SecondaryCommandBuffer& secondary =
        frames[frame].secondary_command_buffers[secondary_buffers_used];
    secondary.render_pass = draw_instruction_list.render_pass;
    secondary.framebuffer = draw_instruction_list.framebuffer;
    secondary.instruction_data.resize(draw_instruction_list.data.size());
    memcpy(secondary.instruction_data.ptr(), draw_instruction_list.data.ptr(),
           draw_instruction_list.data.size());

    // Run a background task for recording the secondary command buffer.
    secondary.task = WorkerThreadPool::get_singleton()->add_template_task(
        this, &RenderingDeviceGraph::_run_secondary_command_buffer_task, &secondary, true);

    // Clear the instruction list and add a single command for executing the secondary command buffer instead.
    draw_instruction_list.data.clear();
    add_draw_list_execute_commands(secondary.command_buffer);
    secondary_buffers_used++;

    command_buffer_type = RDD::COMMAND_BUFFER_TYPE_SECONDARY;
  } else {
    command_buffer_type = RDD::COMMAND_BUFFER_TYPE_PRIMARY;
  }

  int32_t command_index;
  uint32_t clear_values_size =
      sizeof(RDD::RenderPassClearValue) * draw_instruction_list.clear_values.size();
  uint32_t instruction_data_size = draw_instruction_list.data.size();
  uint32_t command_size =
      sizeof(RecordedDrawListCommand) + clear_values_size + instruction_data_size;
  RecordedDrawListCommand* command =
      static_cast<RecordedDrawListCommand*>(_allocate_command(command_size, command_index));
  command->type = RecordedCommand::TYPE_DRAW_LIST;
  command->self_stages = draw_instruction_list.stages;
  command->instruction_data_size = instruction_data_size;
  command->render_pass = draw_instruction_list.render_pass;
  command->framebuffer = draw_instruction_list.framebuffer;
  command->command_buffer_type = command_buffer_type;
  command->region = draw_instruction_list.region;
  command->clear_values_count = draw_instruction_list.clear_values.size();

  RDD::RenderPassClearValue* clear_values = command->clear_values();
  for (uint32_t i = 0; i < command->clear_values_count; i++) {
    clear_values[i] = draw_instruction_list.clear_values[i];
  }

  memcpy(command->instruction_data(), draw_instruction_list.data.ptr(), instruction_data_size);
  _add_command_to_graph(draw_instruction_list.command_trackers.ptr(),
                        draw_instruction_list.command_tracker_usages.ptr(),
                        draw_instruction_list.command_trackers.size(), command_index, command);
}

void RenderingDeviceGraph::add_texture_clear(RDD::TextureID p_dst, ResourceTracker* p_dst_tracker,
                                             const Color& p_color,
                                             const RDD::TextureSubresourceRange& p_range) {
  DEV_ASSERT(p_dst_tracker != nullptr);

  int32_t command_index;
  RecordedTextureClearCommand* command = static_cast<RecordedTextureClearCommand*>(
      _allocate_command(sizeof(RecordedTextureClearCommand), command_index));
  command->type = RecordedCommand::TYPE_TEXTURE_CLEAR;
  command->texture = p_dst;
  command->color = p_color;
  command->range = p_range;

  ResourceUsage usage;
  if (driver_clears_with_copy_engine) {
    command->self_stages = RDD::PIPELINE_STAGE_COPY_BIT;
    usage = RESOURCE_USAGE_COPY_TO;
  } else {
    // If the driver is uncapable of using the copy engine for clearing the image (e.g. D3D12), we must either transition the
    // resource to a render target or a storage image as that's the only two ways it can perform the operation.
    if (p_dst_tracker->texture_usage & RDD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT) {
      command->self_stages = RDD::PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      usage = RESOURCE_USAGE_ATTACHMENT_COLOR_READ_WRITE;
    } else {
      command->self_stages = RDD::PIPELINE_STAGE_CLEAR_STORAGE_BIT;
      usage = RESOURCE_USAGE_STORAGE_IMAGE_READ_WRITE;
    }
  }

  _add_command_to_graph(&p_dst_tracker, &usage, 1, command_index, command);
}

void RenderingDeviceGraph::add_texture_copy(
    RDD::TextureID p_src, ResourceTracker* p_src_tracker, RDD::TextureID p_dst,
    ResourceTracker* p_dst_tracker, VectorView<RDD::TextureCopyRegion> p_texture_copy_regions) {
  DEV_ASSERT(p_src_tracker != nullptr);
  DEV_ASSERT(p_dst_tracker != nullptr);

  int32_t command_index;
  uint64_t command_size = sizeof(RecordedTextureCopyCommand) +
                          p_texture_copy_regions.size() * sizeof(RDD::TextureCopyRegion);
  RecordedTextureCopyCommand* command =
      static_cast<RecordedTextureCopyCommand*>(_allocate_command(command_size, command_index));
  command->type = RecordedCommand::TYPE_TEXTURE_COPY;
  command->self_stages = RDD::PIPELINE_STAGE_COPY_BIT;
  command->from_texture = p_src;
  command->to_texture = p_dst;
  command->texture_copy_regions_count = p_texture_copy_regions.size();

  RDD::TextureCopyRegion* texture_copy_regions = command->texture_copy_regions();
  for (uint32_t i = 0; i < command->texture_copy_regions_count; i++) {
    texture_copy_regions[i] = p_texture_copy_regions[i];
  }

  ResourceTracker* trackers[2] = {p_dst_tracker, p_src_tracker};
  ResourceUsage usages[2] = {RESOURCE_USAGE_COPY_TO, RESOURCE_USAGE_COPY_FROM};
  _add_command_to_graph(trackers, usages, 2, command_index, command);
}

void RenderingDeviceGraph::add_texture_get_data(
    RDD::TextureID p_src, ResourceTracker* p_src_tracker, RDD::BufferID p_dst,
    VectorView<RDD::BufferTextureCopyRegion> p_buffer_texture_copy_regions,
    ResourceTracker* p_dst_tracker) {
  DEV_ASSERT(p_src_tracker != nullptr);

  int32_t command_index;
  uint64_t command_size =
      sizeof(RecordedTextureGetDataCommand) +
      p_buffer_texture_copy_regions.size() * sizeof(RDD::BufferTextureCopyRegion);
  RecordedTextureGetDataCommand* command =
      static_cast<RecordedTextureGetDataCommand*>(_allocate_command(command_size, command_index));
  command->type = RecordedCommand::TYPE_TEXTURE_GET_DATA;
  command->self_stages = RDD::PIPELINE_STAGE_COPY_BIT;
  command->from_texture = p_src;
  command->to_buffer = p_dst;
  command->buffer_texture_copy_regions_count = p_buffer_texture_copy_regions.size();

  RDD::BufferTextureCopyRegion* buffer_texture_copy_regions =
      command->buffer_texture_copy_regions();
  for (uint32_t i = 0; i < command->buffer_texture_copy_regions_count; i++) {
    buffer_texture_copy_regions[i] = p_buffer_texture_copy_regions[i];
  }

  if (p_dst_tracker != nullptr) {
    // Add the optional destination tracker if it was provided.
    ResourceTracker* trackers[2] = {p_dst_tracker, p_src_tracker};
    ResourceUsage usages[2] = {RESOURCE_USAGE_COPY_TO, RESOURCE_USAGE_COPY_FROM};
    _add_command_to_graph(trackers, usages, 2, command_index, command);
  } else {
    ResourceUsage usage = RESOURCE_USAGE_COPY_FROM;
    _add_command_to_graph(&p_src_tracker, &usage, 1, command_index, command);
  }
}

void RenderingDeviceGraph::add_texture_resolve(RDD::TextureID p_src, ResourceTracker* p_src_tracker,
                                               RDD::TextureID p_dst, ResourceTracker* p_dst_tracker,
                                               uint32_t p_src_layer, uint32_t p_src_mipmap,
                                               uint32_t p_dst_layer, uint32_t p_dst_mipmap) {
  DEV_ASSERT(p_src_tracker != nullptr);
  DEV_ASSERT(p_dst_tracker != nullptr);

  int32_t command_index;
  RecordedTextureResolveCommand* command = static_cast<RecordedTextureResolveCommand*>(
      _allocate_command(sizeof(RecordedTextureResolveCommand), command_index));
  command->type = RecordedCommand::TYPE_TEXTURE_RESOLVE;
  command->self_stages = RDD::PIPELINE_STAGE_RESOLVE_BIT;
  command->from_texture = p_src;
  command->to_texture = p_dst;
  command->src_layer = p_src_layer;
  command->src_mipmap = p_src_mipmap;
  command->dst_layer = p_dst_layer;
  command->dst_mipmap = p_dst_mipmap;

  ResourceTracker* trackers[2] = {p_dst_tracker, p_src_tracker};
  ResourceUsage usages[2] = {RESOURCE_USAGE_RESOLVE_TO, RESOURCE_USAGE_RESOLVE_FROM};
  _add_command_to_graph(trackers, usages, 2, command_index, command);
}

void RenderingDeviceGraph::add_texture_update(
    RDD::TextureID p_dst, ResourceTracker* p_dst_tracker,
    VectorView<RecordedBufferToTextureCopy> p_buffer_copies,
    VectorView<ResourceTracker*> p_buffer_trackers) {
  DEV_ASSERT(p_dst_tracker != nullptr);

  int32_t command_index;
  uint64_t command_size = sizeof(RecordedTextureUpdateCommand) +
                          p_buffer_copies.size() * sizeof(RecordedBufferToTextureCopy);
  RecordedTextureUpdateCommand* command =
      static_cast<RecordedTextureUpdateCommand*>(_allocate_command(command_size, command_index));
  command->type = RecordedCommand::TYPE_TEXTURE_UPDATE;
  command->self_stages = RDD::PIPELINE_STAGE_COPY_BIT;
  command->to_texture = p_dst;
  command->buffer_to_texture_copies_count = p_buffer_copies.size();

  RecordedBufferToTextureCopy* buffer_to_texture_copies = command->buffer_to_texture_copies();
  for (uint32_t i = 0; i < command->buffer_to_texture_copies_count; i++) {
    buffer_to_texture_copies[i] = p_buffer_copies[i];
  }

  if (p_buffer_trackers.size() > 0) {
    // Add the optional buffer trackers if they were provided.
    thread_local LocalVector<ResourceTracker*> trackers;
    thread_local LocalVector<ResourceUsage> usages;
    trackers.clear();
    usages.clear();
    for (uint32_t i = 0; i < p_buffer_trackers.size(); i++) {
      trackers.push_back(p_buffer_trackers[i]);
      usages.push_back(RESOURCE_USAGE_COPY_FROM);
    }

    trackers.push_back(p_dst_tracker);
    usages.push_back(RESOURCE_USAGE_COPY_TO);

    _add_command_to_graph(trackers.ptr(), usages.ptr(), trackers.size(), command_index, command);
  } else {
    ResourceUsage usage = RESOURCE_USAGE_COPY_TO;
    _add_command_to_graph(&p_dst_tracker, &usage, 1, command_index, command);
  }
}

void RenderingDeviceGraph::add_capture_timestamp(RDD::QueryPoolID p_query_pool, uint32_t p_index) {
  int32_t command_index;
  RecordedCaptureTimestampCommand* command = static_cast<RecordedCaptureTimestampCommand*>(
      _allocate_command(sizeof(RecordedCaptureTimestampCommand), command_index));
  command->type = RecordedCommand::TYPE_CAPTURE_TIMESTAMP;
  command->self_stages = 0;
  command->pool = p_query_pool;
  command->index = p_index;
  _add_command_to_graph(nullptr, nullptr, 0, command_index, command);
}

void RenderingDeviceGraph::add_synchronization() {
  // Synchronization is only acknowledged if commands have been recorded on the graph already.
  if (command_count > 0) {
    command_synchronization_pending = true;
  }
}

void RenderingDeviceGraph::begin_label(const String& p_label_name, const Color& p_color) {
  uint32_t command_label_offset = command_label_chars.size();
  PackedByteArray command_label_utf8 = p_label_name.to_utf8_buffer();
  int command_label_utf8_size = command_label_utf8.size();
  command_label_chars.resize(command_label_offset + command_label_utf8_size + 1);
  memcpy(&command_label_chars[command_label_offset], command_label_utf8.ptr(),
         command_label_utf8.size());
  command_label_chars[command_label_offset + command_label_utf8_size] = '\0';
  command_label_colors.push_back(p_color);
  command_label_offsets.push_back(command_label_offset);
  command_label_index = command_label_count;
  command_label_count++;
}

void RenderingDeviceGraph::end_label() {
  command_label_index = -1;
}

void RenderingDeviceGraph::_run_secondary_command_buffer_task(
    const SecondaryCommandBuffer* p_secondary) {
  driver->command_buffer_begin_secondary(p_secondary->command_buffer, p_secondary->render_pass, 0,
                                         p_secondary->framebuffer);
  _run_draw_list_command(p_secondary->command_buffer, p_secondary->instruction_data.ptr(),
                         p_secondary->instruction_data.size());
  driver->command_buffer_end(p_secondary->command_buffer);
}

void RenderingDeviceGraph::_run_compute_list_command(RDD::CommandBufferID p_command_buffer,
                                                     const uint8_t* p_instruction_data,
                                                     uint32_t p_instruction_data_size) {
  uint32_t instruction_data_cursor = 0;
  while (instruction_data_cursor < p_instruction_data_size) {
    DEV_ASSERT((instruction_data_cursor + sizeof(ComputeListInstruction)) <=
               p_instruction_data_size);

    const ComputeListInstruction* instruction = reinterpret_cast<const ComputeListInstruction*>(
        &p_instruction_data[instruction_data_cursor]);
    switch (instruction->type) {
      case ComputeListInstruction::TYPE_BIND_PIPELINE: {
        const ComputeListBindPipelineInstruction* bind_pipeline_instruction =
            reinterpret_cast<const ComputeListBindPipelineInstruction*>(instruction);
        driver->command_bind_compute_pipeline(p_command_buffer,
                                              bind_pipeline_instruction->pipeline);
        instruction_data_cursor += sizeof(ComputeListBindPipelineInstruction);
      } break;
      case ComputeListInstruction::TYPE_BIND_UNIFORM_SET: {
        const ComputeListBindUniformSetInstruction* bind_uniform_set_instruction =
            reinterpret_cast<const ComputeListBindUniformSetInstruction*>(instruction);
        driver->command_bind_compute_uniform_set(
            p_command_buffer, bind_uniform_set_instruction->uniform_set,
            bind_uniform_set_instruction->shader, bind_uniform_set_instruction->set_index);
        instruction_data_cursor += sizeof(ComputeListBindUniformSetInstruction);
      } break;
      case ComputeListInstruction::TYPE_DISPATCH: {
        const ComputeListDispatchInstruction* dispatch_instruction =
            reinterpret_cast<const ComputeListDispatchInstruction*>(instruction);
        driver->command_compute_dispatch(p_command_buffer, dispatch_instruction->x_groups,
                                         dispatch_instruction->y_groups,
                                         dispatch_instruction->z_groups);
        instruction_data_cursor += sizeof(ComputeListDispatchInstruction);
      } break;
      case ComputeListInstruction::TYPE_DISPATCH_INDIRECT: {
        const ComputeListDispatchIndirectInstruction* dispatch_indirect_instruction =
            reinterpret_cast<const ComputeListDispatchIndirectInstruction*>(instruction);
        driver->command_compute_dispatch_indirect(p_command_buffer,
                                                  dispatch_indirect_instruction->buffer,
                                                  dispatch_indirect_instruction->offset);
        instruction_data_cursor += sizeof(ComputeListDispatchIndirectInstruction);
      } break;
      case ComputeListInstruction::TYPE_SET_PUSH_CONSTANT: {
        const ComputeListSetPushConstantInstruction* set_push_constant_instruction =
            reinterpret_cast<const ComputeListSetPushConstantInstruction*>(instruction);
        const VectorView push_constant_data_view(
            reinterpret_cast<const uint32_t*>(set_push_constant_instruction->data()),
            set_push_constant_instruction->size / sizeof(uint32_t));
        driver->command_bind_push_constants(p_command_buffer, set_push_constant_instruction->shader,
                                            0, push_constant_data_view);
        instruction_data_cursor += sizeof(ComputeListSetPushConstantInstruction);
        instruction_data_cursor += set_push_constant_instruction->size;
      } break;
        // d3d12需要
      case ComputeListInstruction::TYPE_UNIFORM_SET_PREPARE_FOR_USE: {
        const ComputeListUniformSetPrepareForUseInstruction*
            uniform_set_prepare_for_use_instruction =
                reinterpret_cast<const ComputeListUniformSetPrepareForUseInstruction*>(instruction);
        // driver->command_uniform_set_prepare_for_use(p_command_buffer, uniform_set_prepare_for_use_instruction->uniform_set, uniform_set_prepare_for_use_instruction->shader, uniform_set_prepare_for_use_instruction->set_index);
        instruction_data_cursor += sizeof(ComputeListUniformSetPrepareForUseInstruction);
      } break;
      default:
        DEV_ASSERT(false && "Unknown compute list instruction type.");
        return;
    }
  }
}
// 感觉这里的设计是比较糙的，这样每次增加一个字段需要在好多地方更改
// 方案：使用虚函数，代价是多4个字节（虚函数表）
// @todo 测试。 考虑到PSO占用内存很大，未必好用。
// 可以考虑最近大家研究的proxy机制（通过concept？）
/* 
while(instruction_data_cursor < p_instruction_data_size) {
  const DrawListInstruction *instruction = reinterpret_cast<const DrawListInstruction *>(&p_instruction_data[instruction_data_cursor]);
  instruction->register_to_command_buffer(p_command_buffer);
  instruction_data_cursor += instruction->length();
}
*/
// 执行（开始向缓冲区中记录命令，然后提交给GPU） 在workerthread中执行

void RenderingDeviceGraph::_run_draw_list_command(RDD::CommandBufferID p_command_buffer,
                                                  const uint8_t* p_instruction_data,
                                                  uint32_t p_instruction_data_size) {
  uint32_t instruction_data_cursor = 0;
  while (instruction_data_cursor < p_instruction_data_size) {
    DEV_ASSERT((instruction_data_cursor + sizeof(DrawListInstruction)) <= p_instruction_data_size);

    const DrawListInstruction* instruction =
        reinterpret_cast<const DrawListInstruction*>(&p_instruction_data[instruction_data_cursor]);
    switch (instruction->type) {
      case DrawListInstruction::TYPE_BIND_INDEX_BUFFER: {
        const DrawListBindIndexBufferInstruction* bind_index_buffer_instruction =
            reinterpret_cast<const DrawListBindIndexBufferInstruction*>(instruction);
        driver->command_render_bind_index_buffer(
            p_command_buffer, bind_index_buffer_instruction->buffer,
            bind_index_buffer_instruction->format, bind_index_buffer_instruction->offset);
        instruction_data_cursor += sizeof(DrawListBindIndexBufferInstruction);
      } break;
      case DrawListInstruction::TYPE_BIND_PIPELINE: {
        const DrawListBindPipelineInstruction* bind_pipeline_instruction =
            reinterpret_cast<const DrawListBindPipelineInstruction*>(instruction);
        driver->command_bind_render_pipeline(p_command_buffer, bind_pipeline_instruction->pipeline);
        instruction_data_cursor += sizeof(DrawListBindPipelineInstruction);
      } break;
      case DrawListInstruction::TYPE_BIND_UNIFORM_SET: {
        const DrawListBindUniformSetInstruction* bind_uniform_set_instruction =
            reinterpret_cast<const DrawListBindUniformSetInstruction*>(instruction);
        driver->command_bind_render_uniform_set(
            p_command_buffer, bind_uniform_set_instruction->uniform_set,
            bind_uniform_set_instruction->shader, bind_uniform_set_instruction->set_index);
        instruction_data_cursor += sizeof(DrawListBindUniformSetInstruction);
      } break;
      case DrawListInstruction::TYPE_BIND_VERTEX_BUFFERS: {
        const DrawListBindVertexBuffersInstruction* bind_vertex_buffers_instruction =
            reinterpret_cast<const DrawListBindVertexBuffersInstruction*>(instruction);
        driver->command_render_bind_vertex_buffers(
            p_command_buffer, bind_vertex_buffers_instruction->vertex_buffers_count,
            bind_vertex_buffers_instruction->vertex_buffers(),
            bind_vertex_buffers_instruction->vertex_buffer_offsets());
        instruction_data_cursor += sizeof(DrawListBindVertexBuffersInstruction);
        instruction_data_cursor +=
            sizeof(RDD::BufferID) * bind_vertex_buffers_instruction->vertex_buffers_count;
        instruction_data_cursor +=
            sizeof(uint64_t) * bind_vertex_buffers_instruction->vertex_buffers_count;
      } break;
      case DrawListInstruction::TYPE_CLEAR_ATTACHMENTS: {
        const DrawListClearAttachmentsInstruction* clear_attachments_instruction =
            reinterpret_cast<const DrawListClearAttachmentsInstruction*>(instruction);
        const VectorView attachments_clear_view(
            clear_attachments_instruction->attachments_clear(),
            clear_attachments_instruction->attachments_clear_count);
        const VectorView attachments_clear_rect_view(
            clear_attachments_instruction->attachments_clear_rect(),
            clear_attachments_instruction->attachments_clear_rect_count);
        driver->command_render_clear_attachments(p_command_buffer, attachments_clear_view,
                                                 attachments_clear_rect_view);
        instruction_data_cursor += sizeof(DrawListClearAttachmentsInstruction);
        instruction_data_cursor +=
            sizeof(RDD::AttachmentClear) * clear_attachments_instruction->attachments_clear_count;
        instruction_data_cursor +=
            sizeof(Rect2i) * clear_attachments_instruction->attachments_clear_rect_count;
      } break;
      case DrawListInstruction::TYPE_DRAW: {
        const DrawListDrawInstruction* draw_instruction =
            reinterpret_cast<const DrawListDrawInstruction*>(instruction);
        driver->command_render_draw(p_command_buffer, draw_instruction->vertex_count,
                                    draw_instruction->instance_count, 0, 0);
        instruction_data_cursor += sizeof(DrawListDrawInstruction);
      } break;
      case DrawListInstruction::TYPE_DRAW_INDEXED: {
        const DrawListDrawIndexedInstruction* draw_indexed_instruction =
            reinterpret_cast<const DrawListDrawIndexedInstruction*>(instruction);
        driver->command_render_draw_indexed(p_command_buffer, draw_indexed_instruction->index_count,
                                            draw_indexed_instruction->instance_count,
                                            draw_indexed_instruction->first_index, 0, 0);
        instruction_data_cursor += sizeof(DrawListDrawIndexedInstruction);
      } break;
      case DrawListInstruction::TYPE_EXECUTE_COMMANDS: {
        const DrawListExecuteCommandsInstruction* execute_commands_instruction =
            reinterpret_cast<const DrawListExecuteCommandsInstruction*>(instruction);
        driver->command_buffer_execute_secondary(p_command_buffer,
                                                 execute_commands_instruction->command_buffer);
        instruction_data_cursor += sizeof(DrawListExecuteCommandsInstruction);
      } break;
      case DrawListInstruction::TYPE_NEXT_SUBPASS: {
        const DrawListNextSubpassInstruction* next_subpass_instruction =
            reinterpret_cast<const DrawListNextSubpassInstruction*>(instruction);
        driver->command_next_render_subpass(p_command_buffer,
                                            next_subpass_instruction->command_buffer_type);
        instruction_data_cursor += sizeof(DrawListNextSubpassInstruction);
      } break;
      case DrawListInstruction::TYPE_SET_BLEND_CONSTANTS: {
        const DrawListSetBlendConstantsInstruction* set_blend_constants_instruction =
            reinterpret_cast<const DrawListSetBlendConstantsInstruction*>(instruction);
        driver->command_render_set_blend_constants(p_command_buffer,
                                                   set_blend_constants_instruction->color);
        instruction_data_cursor += sizeof(DrawListSetBlendConstantsInstruction);
      } break;
      case DrawListInstruction::TYPE_SET_LINE_WIDTH: {
        const DrawListSetLineWidthInstruction* set_line_width_instruction =
            reinterpret_cast<const DrawListSetLineWidthInstruction*>(instruction);
        driver->command_render_set_line_width(p_command_buffer, set_line_width_instruction->width);
        instruction_data_cursor += sizeof(DrawListSetLineWidthInstruction);
      } break;
      case DrawListInstruction::TYPE_SET_PUSH_CONSTANT: {
        const DrawListSetPushConstantInstruction* set_push_constant_instruction =
            reinterpret_cast<const DrawListSetPushConstantInstruction*>(instruction);
        const VectorView push_constant_data_view(
            reinterpret_cast<const uint32_t*>(set_push_constant_instruction->data()),
            set_push_constant_instruction->size / sizeof(uint32_t));
        driver->command_bind_push_constants(p_command_buffer, set_push_constant_instruction->shader,
                                            0, push_constant_data_view);
        instruction_data_cursor += sizeof(DrawListSetPushConstantInstruction);
        instruction_data_cursor += set_push_constant_instruction->size;
      } break;
      case DrawListInstruction::TYPE_SET_SCISSOR: {
        const DrawListSetScissorInstruction* set_scissor_instruction =
            reinterpret_cast<const DrawListSetScissorInstruction*>(instruction);
        driver->command_render_set_scissor(p_command_buffer, set_scissor_instruction->rect);
        instruction_data_cursor += sizeof(DrawListSetScissorInstruction);
      } break;
      case DrawListInstruction::TYPE_SET_VIEWPORT: {
        const DrawListSetViewportInstruction* set_viewport_instruction =
            reinterpret_cast<const DrawListSetViewportInstruction*>(instruction);
        driver->command_render_set_viewport(p_command_buffer, set_viewport_instruction->rect);
        instruction_data_cursor += sizeof(DrawListSetViewportInstruction);
      } break;
      case DrawListInstruction::TYPE_UNIFORM_SET_PREPARE_FOR_USE: {
        const DrawListUniformSetPrepareForUseInstruction* uniform_set_prepare_for_use_instruction =
            reinterpret_cast<const DrawListUniformSetPrepareForUseInstruction*>(instruction);
        // driver->command_uniform_set_prepare_for_use(p_command_buffer, uniform_set_prepare_for_use_instruction->uniform_set, uniform_set_prepare_for_use_instruction->shader, uniform_set_prepare_for_use_instruction->set_index);
        instruction_data_cursor += sizeof(DrawListUniformSetPrepareForUseInstruction);
      } break;
      default:
        DEV_ASSERT(false && "Unknown draw list instruction type.");
        return;
    }
  }
}

void RenderingDeviceGraph::_run_render_command(
    int32_t p_level, const RecordedCommandSort* p_sorted_commands, uint32_t p_sorted_commands_count,
    RDD::CommandBufferID& r_command_buffer, CommandBufferPool& r_command_buffer_pool,
    int32_t& r_current_label_index, int32_t& r_current_label_level) {
  for (uint32_t i = 0; i < p_sorted_commands_count; i++) {
    const RecordedCommand* command = _get_command(p_sorted_commands[i].index);
    _run_label_command_change(r_command_buffer, command->label_index, p_level, false, true,
                              &p_sorted_commands[i], p_sorted_commands_count - i,
                              r_current_label_index, r_current_label_level);

    switch (command->type) {
      case RecordedCommand::TYPE_BUFFER_CLEAR: {
        const RecordedBufferClearCommand* buffer_clear_command =
            reinterpret_cast<const RecordedBufferClearCommand*>(command);
        driver->command_clear_buffer(r_command_buffer, buffer_clear_command->buffer,
                                     buffer_clear_command->offset, buffer_clear_command->size);
      } break;
      case RecordedCommand::TYPE_BUFFER_COPY: {
        const RecordedBufferCopyCommand* buffer_copy_command =
            reinterpret_cast<const RecordedBufferCopyCommand*>(command);
        driver->command_copy_buffer(r_command_buffer, buffer_copy_command->source,
                                    buffer_copy_command->destination, buffer_copy_command->region);
      } break;
      case RecordedCommand::TYPE_BUFFER_GET_DATA: {
        const RecordedBufferGetDataCommand* buffer_get_data_command =
            reinterpret_cast<const RecordedBufferGetDataCommand*>(command);
        driver->command_copy_buffer(r_command_buffer, buffer_get_data_command->source,
                                    buffer_get_data_command->destination,
                                    buffer_get_data_command->region);
      } break;
      case RecordedCommand::TYPE_BUFFER_UPDATE: {
        const RecordedBufferUpdateCommand* buffer_update_command =
            reinterpret_cast<const RecordedBufferUpdateCommand*>(command);
        const RecordedBufferCopy* command_buffer_copies = buffer_update_command->buffer_copies();
        for (uint32_t j = 0; j < buffer_update_command->buffer_copies_count; j++) {
          driver->command_copy_buffer(r_command_buffer, command_buffer_copies[j].source,
                                      buffer_update_command->destination,
                                      command_buffer_copies[j].region);
        }
      } break;
      case RecordedCommand::TYPE_COMPUTE_LIST: {
        // Adreno 6xx会有问题，很神奇，这也能发现
        // if (device.workarounds.avoid_compute_after_draw && workarounds_state.draw_list_found) {
        //   // Avoid compute after draw workaround. Refer to the comment that enables this in the Vulkan driver for more information.
        //   workarounds_state.draw_list_found = false;

        //   // Create or reuse a command buffer and finish recording the current one.
        //   driver->command_buffer_end(r_command_buffer);

        //   while (r_command_buffer_pool.buffers_used >= r_command_buffer_pool.buffers.size()) {
        //     RDD::CommandBufferID command_buffer =
        //         driver->command_buffer_create(r_command_buffer_pool.pool);
        //     RDD::SemaphoreID command_semaphore = driver->semaphore_create();
        //     r_command_buffer_pool.buffers.push_back(command_buffer);
        //     r_command_buffer_pool.semaphores.push_back(command_semaphore);
        //   }

        //   // Start recording on the next usable command buffer from the pool.
        //   uint32_t command_buffer_index = r_command_buffer_pool.buffers_used++;
        //   r_command_buffer = r_command_buffer_pool.buffers[command_buffer_index];
        //   driver->command_buffer_begin(r_command_buffer);
        // }

        const RecordedComputeListCommand* compute_list_command =
            reinterpret_cast<const RecordedComputeListCommand*>(command);
        _run_compute_list_command(r_command_buffer, compute_list_command->instruction_data(),
                                  compute_list_command->instruction_data_size);
      } break;
      case RecordedCommand::TYPE_DRAW_LIST: {
        // if (device.workarounds.avoid_compute_after_draw) {
        //   // Indicate that a draw list was encountered for the workaround.
        //   workarounds_state.draw_list_found = true;
        // }

        const RecordedDrawListCommand* draw_list_command =
            reinterpret_cast<const RecordedDrawListCommand*>(command);
        const VectorView clear_values(draw_list_command->clear_values(),
                                      draw_list_command->clear_values_count);
        driver->command_begin_render_pass(
            r_command_buffer, draw_list_command->render_pass, draw_list_command->framebuffer,
            draw_list_command->command_buffer_type, draw_list_command->region, clear_values);
        _run_draw_list_command(r_command_buffer, draw_list_command->instruction_data(),
                               draw_list_command->instruction_data_size);
        driver->command_end_render_pass(r_command_buffer);
      } break;
      case RecordedCommand::TYPE_TEXTURE_CLEAR: {
        const RecordedTextureClearCommand* texture_clear_command =
            reinterpret_cast<const RecordedTextureClearCommand*>(command);
        driver->command_clear_color_texture(
            r_command_buffer, texture_clear_command->texture, RDD::TEXTURE_LAYOUT_COPY_DST_OPTIMAL,
            texture_clear_command->color, texture_clear_command->range);
      } break;
      case RecordedCommand::TYPE_TEXTURE_COPY: {
        const RecordedTextureCopyCommand* texture_copy_command =
            reinterpret_cast<const RecordedTextureCopyCommand*>(command);
        const VectorView<RDD::TextureCopyRegion> command_texture_copy_regions_view(
            texture_copy_command->texture_copy_regions(),
            texture_copy_command->texture_copy_regions_count);
        driver->command_copy_texture(
            r_command_buffer, texture_copy_command->from_texture,
            RDD::TEXTURE_LAYOUT_COPY_SRC_OPTIMAL, texture_copy_command->to_texture,
            RDD::TEXTURE_LAYOUT_COPY_DST_OPTIMAL, command_texture_copy_regions_view);
      } break;
      case RecordedCommand::TYPE_TEXTURE_GET_DATA: {
        const RecordedTextureGetDataCommand* texture_get_data_command =
            reinterpret_cast<const RecordedTextureGetDataCommand*>(command);
        const VectorView<RDD::BufferTextureCopyRegion> command_buffer_texture_copy_regions_view(
            texture_get_data_command->buffer_texture_copy_regions(),
            texture_get_data_command->buffer_texture_copy_regions_count);
        driver->command_copy_texture_to_buffer(
            r_command_buffer, texture_get_data_command->from_texture,
            RDD::TEXTURE_LAYOUT_COPY_SRC_OPTIMAL, texture_get_data_command->to_buffer,
            command_buffer_texture_copy_regions_view);
      } break;
      case RecordedCommand::TYPE_TEXTURE_RESOLVE: {
        const RecordedTextureResolveCommand* texture_resolve_command =
            reinterpret_cast<const RecordedTextureResolveCommand*>(command);
        driver->command_resolve_texture(
            r_command_buffer, texture_resolve_command->from_texture,
            RDD::TEXTURE_LAYOUT_RESOLVE_SRC_OPTIMAL, texture_resolve_command->src_layer,
            texture_resolve_command->src_mipmap, texture_resolve_command->to_texture,
            RDD::TEXTURE_LAYOUT_RESOLVE_DST_OPTIMAL, texture_resolve_command->dst_layer,
            texture_resolve_command->dst_mipmap);
      } break;
      case RecordedCommand::TYPE_TEXTURE_UPDATE: {
        const RecordedTextureUpdateCommand* texture_update_command =
            reinterpret_cast<const RecordedTextureUpdateCommand*>(command);
        const RecordedBufferToTextureCopy* command_buffer_to_texture_copies =
            texture_update_command->buffer_to_texture_copies();
        for (uint32_t j = 0; j < texture_update_command->buffer_to_texture_copies_count; j++) {
          driver->command_copy_buffer_to_texture(
              r_command_buffer, command_buffer_to_texture_copies[j].from_buffer,
              texture_update_command->to_texture, RDD::TEXTURE_LAYOUT_COPY_DST_OPTIMAL,
              command_buffer_to_texture_copies[j].region);
        }
      } break;
      case RecordedCommand::TYPE_CAPTURE_TIMESTAMP: {
        const RecordedCaptureTimestampCommand* texture_capture_timestamp_command =
            reinterpret_cast<const RecordedCaptureTimestampCommand*>(command);
        driver->command_timestamp_write(r_command_buffer, texture_capture_timestamp_command->pool,
                                        texture_capture_timestamp_command->index);
      } break;
      default: {
        DEV_ASSERT(false && "Unknown recorded command type.");
        return;
      }
    }
  }
}

void RenderingDeviceGraph::_run_label_command_change(
    RDD::CommandBufferID p_command_buffer, int32_t p_new_label_index, int32_t p_new_level,
    bool p_ignore_previous_value, bool p_use_label_for_empty,
    const RecordedCommandSort* p_sorted_commands, uint32_t p_sorted_commands_count,
    int32_t& r_current_label_index, int32_t& r_current_label_level) {
  if (command_label_count == 0) {
    // Ignore any label operations if no labels were pushed.
    return;
  }

  if (p_ignore_previous_value || p_new_label_index != r_current_label_index ||
      p_new_level != r_current_label_level) {
    if (!p_ignore_previous_value && (p_use_label_for_empty || r_current_label_index >= 0)) {
      // End the current label.
      driver->command_end_label(p_command_buffer);
    }

    String label_name;
    Color label_color;
    if (p_new_label_index >= 0) {
      const char* label_chars = &command_label_chars[command_label_offsets[p_new_label_index]];
      label_name.parse_utf8(label_chars);
      label_color = command_label_colors[p_new_label_index];
    } else if (p_use_label_for_empty) {
      label_name = "Command graph";
      label_color = Color(1, 1, 1, 1);
    }

    // Add the level to the name.
    label_name += " (L" + itos(p_new_level) + ")";

    if (p_sorted_commands != nullptr && p_sorted_commands_count > 0) {
      // Analyze the commands in the level that have the same label to detect what type of operations are performed.
      bool copy_commands = false;
      bool compute_commands = false;
      bool draw_commands = false;
      for (uint32_t i = 0; i < p_sorted_commands_count; i++) {
        const uint32_t command_index = p_sorted_commands[i].index;
        const uint32_t command_data_offset = command_data_offsets[command_index];
        const RecordedCommand* command =
            reinterpret_cast<RecordedCommand*>(&command_data[command_data_offset]);
        if (command->label_index != p_new_label_index) {
          break;
        }

        switch (command->type) {
          case RecordedCommand::TYPE_BUFFER_CLEAR:
          case RecordedCommand::TYPE_BUFFER_COPY:
          case RecordedCommand::TYPE_BUFFER_GET_DATA:
          case RecordedCommand::TYPE_BUFFER_UPDATE:
          case RecordedCommand::TYPE_TEXTURE_CLEAR:
          case RecordedCommand::TYPE_TEXTURE_COPY:
          case RecordedCommand::TYPE_TEXTURE_GET_DATA:
          case RecordedCommand::TYPE_TEXTURE_RESOLVE:
          case RecordedCommand::TYPE_TEXTURE_UPDATE: {
            copy_commands = true;
          } break;
          case RecordedCommand::TYPE_COMPUTE_LIST: {
            compute_commands = true;
          } break;
          case RecordedCommand::TYPE_DRAW_LIST: {
            draw_commands = true;
          } break;
          default: {
            // Ignore command.
          } break;
        }

        if (copy_commands && compute_commands && draw_commands) {
          // There's no more command types to find.
          break;
        }
      }

      if (copy_commands || compute_commands || draw_commands) {
        // Add the operations to the name.
        bool plus_after_copy = copy_commands && (compute_commands || draw_commands);
        bool plus_after_compute = compute_commands && draw_commands;
        label_name += " (";
        label_name += copy_commands ? "Copy" : "";
        label_name += plus_after_copy ? "+" : "";
        label_name += compute_commands ? "Compute" : "";
        label_name += plus_after_compute ? "+" : "";
        label_name += draw_commands ? "Draw" : "";
        label_name += ")";
      }
    }

    // Start the new label.

    driver->command_begin_label(p_command_buffer, label_name.utf8().get_data(), label_color);

    r_current_label_index = p_new_label_index;
    r_current_label_level = p_new_level;
  }
}
void RenderingDeviceGraph::begin() {
  command_data.clear();
  command_data_offsets.clear();
  command_normalization_barriers.clear();
  command_transition_barriers.clear();
  command_buffer_barriers.clear();
  command_label_chars.clear();
  command_label_colors.clear();
  command_label_offsets.clear();
  command_list_nodes.clear();
  read_slice_list_nodes.clear();
  write_slice_list_nodes.clear();
  command_count = 0;
  command_label_count = 0;
  command_timestamp_index = -1;
  command_synchronization_index = -1;
  command_synchronization_pending = false;
  command_label_index = -1;
  frames[frame].secondary_command_buffers_used = 0;
  draw_instruction_list.index = 0;
  compute_instruction_list.index = 0;
  tracking_frame++;

#ifdef DEV_ENABLED
  write_dependency_counters.clear();
#endif
}

const uint32_t RenderingDeviceGraph::RecordedCommandSort::PriorityTable[] = {
    0,  // TYPE_NONE
    1,  // TYPE_BUFFER_CLEAR
    1,  // TYPE_BUFFER_COPY
    1,  // TYPE_BUFFER_GET_DATA
    1,  // TYPE_BUFFER_UPDATE
    4,  // TYPE_COMPUTE_LIST
    3,  // TYPE_DRAW_LIST
    2,  // TYPE_TEXTURE_CLEAR
    2,  // TYPE_TEXTURE_COPY
    2,  // TYPE_TEXTURE_GET_DATA
    2,  // TYPE_TEXTURE_RESOLVE
    2,  // TYPE_TEXTURE_UPDATE
};

void RenderingDeviceGraph::end(bool p_reorder_commands, bool p_full_barriers,
                               RDD::CommandBufferID& r_command_buffer,
                               CommandBufferPool& r_command_buffer_pool) {
  if (command_count == 0)
    return;
  thread_local LocalVector<RecordedCommandSort> commands_sorted;

  if (p_reorder_commands) {  //
    // 计算每个顶点的入度，从每个根节点进行BFS遍历
    // 为什么都使用thread_local?
    thread_local LocalVector<uint32_t> command_degrees;
    command_degrees.resize(command_count);
    // 必须初始化为0，否则有错
		memset(command_degrees.ptr(), 0, sizeof(uint32_t) * command_degrees.size());
    // 这两个为什么不是先后的？明明是读了一个texture

    for (int32_t i = 0; i < command_count; i++) {
      const RecordedCommand* recorded_command = _get_command(i);
      L_PRINT("Type:", recorded_command->type);
      int32_t adjcent_list_idx = recorded_command->adjacent_command_list_index;
      while (adjcent_list_idx >= 0) {
        const RecordedCommandListNode& command_list_node = command_list_nodes[adjcent_list_idx];
        DEV_ASSERT((command_list_node.command_index != int32_t(i)) &&
                   "Command can't have itself as a dependency.");
        command_degrees[command_list_node.command_index] += 1;
        adjcent_list_idx = command_list_node.next_list_index;
      }
    }
    // 根节点
    thread_local LocalVector<int32_t> command_stack;
    command_stack.clear();
    for (int32_t i = 0; i < command_count; i++) {
      if (command_degrees[i] == 0) {
        command_stack.push_back(i);
      }
    }
    thread_local LocalVector<int32_t> sorted_command_indices;
    // BFS
    sorted_command_indices.clear();
    int32_t adjacent_list_idx = 0;  // 用于遍历邻接表

    while (!command_stack.is_empty()) {  // 层序遍历
      const RecordedCommand* recorded_command = _get_command(command_stack.back());
      sorted_command_indices.push_back(command_stack.back());
      command_stack.resize(command_stack.size() - 1);
      adjacent_list_idx = recorded_command->adjacent_command_list_index;
      while (adjacent_list_idx >= 0) {
        const RecordedCommandListNode& command_list_node = command_list_nodes[adjacent_list_idx];
        // 入栈
        uint32_t& command_degree = command_degrees[command_list_node.command_index];
        command_degree -= 1;
        if (command_degree == 0) {  // 说明就在这一层（最深的父亲）
          command_stack.push_back(command_list_node.command_index);
        }
        adjacent_list_idx = command_list_node.next_list_index;
      }
    }

    commands_sorted.clear();
    commands_sorted.resize(command_count);
    // 填写commands_sorted的index和level，根据sorted_command_indices
    for (int32_t i = 0; i < command_count; i++) {
      int32_t sorted_command_index = sorted_command_indices[i];
      const RecordedCommand* recorded_command = _get_command(sorted_command_index);
      const uint32_t next_command_level = commands_sorted[sorted_command_index].level + 1;
      adjacent_list_idx = recorded_command->adjacent_command_list_index;
      // 更新所有儿子的层，取更大的
      while (adjacent_list_idx >= 0) {
        const RecordedCommandListNode& command_list_node = command_list_nodes[adjacent_list_idx];
        uint32_t& adjacent_command_level = commands_sorted[command_list_node.command_index].level;
        if (adjacent_command_level < next_command_level) {
          adjacent_command_level = next_command_level;
        }
        adjacent_list_idx = command_list_node.next_list_index;
      }
      commands_sorted[sorted_command_index].index = sorted_command_index;
      commands_sorted[sorted_command_index].priority =
          RecordedCommandSort::PriorityTable[recorded_command->type];
    }

  }  // if(p_reorder_commands)
  else {
    commands_sorted.clear();
    commands_sorted.resize(command_count);
    for (int i = 0; i < command_count; i++) {
      commands_sorted[i].index = i;
    }
  }
  _wait_for_secondary_command_buffer_tasks();
  int32_t current_label_index = -1;
  int32_t current_label_level = -1;
  // reset label
  _run_label_command_change(r_command_buffer, -1, -1, true, true, nullptr, 0, current_label_index,
                            current_label_level);
  if (p_reorder_commands) {
#if PRINT_RENDER_GRAPH
    print_line("BEFORE SORT");
    _print_render_commands(commands_sorted.ptr(), command_count);
#endif

    commands_sorted.sort();

#if PRINT_RENDER_GRAPH
    print_line("AFTER SORT");  // 每一层根据优先级排序
    _print_render_commands(commands_sorted.ptr(), command_count);
#endif

#if PRINT_COMMAND_RECORDING
    print_line(vformat("Recording %d commands", command_count));
#endif
    uint32_t boosted_priority = 0;
    uint32_t current_level = commands_sorted[0].level;
    uint32_t current_level_start = 0;
    for (uint32_t i = 0; i < command_count; i++) {
      if (current_level != commands_sorted[i].level) {  // new level
        RecordedCommandSort* level_command_ptr = &commands_sorted[current_level_start];
        uint32_t level_command_count = i - current_level_start;
        _boost_priority_for_render_commands(level_command_ptr, level_command_count,
                                            boosted_priority);
        _group_barriers_for_render_commands(r_command_buffer, level_command_ptr,
                                            level_command_count, p_full_barriers);
        _run_render_command(current_level, level_command_ptr, level_command_count, r_command_buffer,
                            r_command_buffer_pool, current_label_index, current_label_level);
        current_level = commands_sorted[i].level;
        current_level_start = i;
      }
    }
    // 最后一层
    RecordedCommandSort* level_command_ptr = &commands_sorted[current_level_start];
    uint32_t level_command_count = command_count - current_level_start;
    _boost_priority_for_render_commands(level_command_ptr, level_command_count, boosted_priority);
    _group_barriers_for_render_commands(r_command_buffer, level_command_ptr, level_command_count,
                                        p_full_barriers);
    _run_render_command(current_level, level_command_ptr, level_command_count, r_command_buffer,
                        r_command_buffer_pool, current_label_index, current_label_level);

#if PRINT_RENDER_GRAPH
    print_line("COMMANDS", command_count, "LEVELS", current_level + 1);
#endif

  } else {
    for (uint32_t i = 0; i < command_count; i++) {
      // 挨个加barrier
      _group_barriers_for_render_commands(r_command_buffer, &commands_sorted[i], 1,
                                          p_full_barriers);
      _run_render_command(i, &commands_sorted[i], 1, r_command_buffer, r_command_buffer_pool,
                          current_label_index, current_label_level);
    }
  }
  _run_label_command_change(r_command_buffer, -1, -1, true, false, nullptr, 0, current_label_index,
                            current_label_level);

  frame = (frame + 1) % frames.size();
}

void RenderingDeviceGraph::_boost_priority_for_render_commands(
    RecordedCommandSort* p_sorted_commands, uint32_t p_sorted_commands_count,
    uint32_t& r_boosted_priority) {
  if (p_sorted_commands_count == 0) {
    return;
  }

  const uint32_t boosted_priority_value = 0;
  if (r_boosted_priority > 0) {
    bool perform_sort = false;
    for (uint32_t j = 0; j < p_sorted_commands_count; j++) {
      if (p_sorted_commands[j].priority == r_boosted_priority) {
        p_sorted_commands[j].priority = boosted_priority_value;
        perform_sort = true;
      }
    }

    if (perform_sort) {
      SortArray<RecordedCommandSort> command_sorter;
      command_sorter.sort(p_sorted_commands, p_sorted_commands_count);
    }
  }
  // 最后一个指令的优先级；将与该优先级相同的指令设置为0 （为什么？以方便下次执行吗）
  // 
  if (p_sorted_commands[p_sorted_commands_count - 1].priority != boosted_priority_value) {
    r_boosted_priority = p_sorted_commands[p_sorted_commands_count - 1].priority;
  }
}

void RenderingDeviceGraph::_group_barriers_for_render_commands(
    RDD::CommandBufferID p_command_buffer, const RecordedCommandSort* p_sorted_commands,
    uint32_t p_sorted_commands_count, bool p_full_memory_barrier) {
  if (!driver_honors_barriers) {
    return;
  }

  barrier_group.clear();
  barrier_group.src_stages = RDD::PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  barrier_group.dst_stages = RDD::PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

  for (uint32_t i = 0; i < p_sorted_commands_count; i++) {
    const RecordedCommand* command = _get_command(p_sorted_commands[i].index);

#if PRINT_COMMAND_RECORDING
    print_line(vformat("Grouping barriers for #%d", p_sorted_commands[i].index));
#endif

    // Merge command's stage bits with the barrier group.
    barrier_group.src_stages = barrier_group.src_stages | command->previous_stages;
    barrier_group.dst_stages = barrier_group.dst_stages | command->next_stages;

    // Merge command's memory barrier bits with the barrier group.
    barrier_group.memory_barrier.src_access =
        barrier_group.memory_barrier.src_access | command->memory_barrier.src_access;
    barrier_group.memory_barrier.dst_access =
        barrier_group.memory_barrier.dst_access | command->memory_barrier.dst_access;

    // Gather texture barriers.
    for (int32_t j = 0; j < command->normalization_barrier_count; j++) {
      const RDD::TextureBarrier& recorded_barrier =
          command_normalization_barriers[command->normalization_barrier_index + j];
      barrier_group.normalization_barriers.push_back(recorded_barrier);
#if PRINT_COMMAND_RECORDING
      print_line(
          vformat("Normalization Barrier #%d", barrier_group.normalization_barriers.size() - 1));
#endif
    }

    for (int32_t j = 0; j < command->transition_barrier_count; j++) {
      const RDD::TextureBarrier& recorded_barrier =
          command_transition_barriers[command->transition_barrier_index + j];
      barrier_group.transition_barriers.push_back(recorded_barrier);
#if PRINT_COMMAND_RECORDING
      print_line(vformat("Transition Barrier #%d", barrier_group.transition_barriers.size() - 1));
#endif
    }

#if USE_BUFFER_BARRIERS
    // Gather buffer barriers.
    for (int32_t j = 0; j < command->buffer_barrier_count; j++) {
      const RDD::BufferBarrier& recorded_barrier =
          command_buffer_barriers[command->buffer_barrier_index + j];
      barrier_group.buffer_barriers.push_back(recorded_barrier);
    }
#endif
  }

  if (p_full_memory_barrier) {
    barrier_group.src_stages = RDD::PIPELINE_STAGE_ALL_COMMANDS_BIT;
    barrier_group.dst_stages = RDD::PIPELINE_STAGE_ALL_COMMANDS_BIT;
    barrier_group.memory_barrier.src_access =
        RDD::BARRIER_ACCESS_MEMORY_READ_BIT | RDD::BARRIER_ACCESS_MEMORY_WRITE_BIT;
    barrier_group.memory_barrier.dst_access =
        RDD::BARRIER_ACCESS_MEMORY_READ_BIT | RDD::BARRIER_ACCESS_MEMORY_WRITE_BIT;
  }

  const bool is_memory_barrier_empty = barrier_group.memory_barrier.src_access.is_empty() &&
                                       barrier_group.memory_barrier.dst_access.is_empty();
  const bool are_texture_barriers_empty = barrier_group.normalization_barriers.is_empty() &&
                                          barrier_group.transition_barriers.is_empty();
#if USE_BUFFER_BARRIERS
  const bool are_buffer_barriers_empty = barrier_group.buffer_barriers.is_empty();
#else
  const bool are_buffer_barriers_empty = true;
#endif
  if (is_memory_barrier_empty && are_texture_barriers_empty && are_buffer_barriers_empty) {
    // Commands don't require synchronization.
    return;
  }

  const VectorView<RDD::MemoryBarrier> memory_barriers =
      !is_memory_barrier_empty ? barrier_group.memory_barrier : VectorView<RDD::MemoryBarrier>();
  const VectorView<RDD::TextureBarrier> texture_barriers =
      barrier_group.normalization_barriers.is_empty() ? barrier_group.transition_barriers
                                                      : barrier_group.normalization_barriers;
#if USE_BUFFER_BARRIERS
  const VectorView<RDD::BufferBarrier> buffer_barriers =
      !are_buffer_barriers_empty ? barrier_group.buffer_barriers : VectorView<RDD::BufferBarrier>();
#else
  const VectorView<RDD::BufferBarrier> buffer_barriers = VectorView<RDD::BufferBarrier>();
#endif

  driver->command_pipeline_barrier(p_command_buffer, barrier_group.src_stages,
                                   barrier_group.dst_stages, memory_barriers, buffer_barriers,
                                   texture_barriers);

  bool separate_texture_barriers = !barrier_group.normalization_barriers.is_empty() &&
                                   !barrier_group.transition_barriers.is_empty();
  if (separate_texture_barriers) {
    driver->command_pipeline_barrier(p_command_buffer, barrier_group.src_stages,
                                     barrier_group.dst_stages, VectorView<RDD::MemoryBarrier>(),
                                     VectorView<RDD::BufferBarrier>(),
                                     barrier_group.transition_barriers);
  }
}

void RenderingDeviceGraph::_wait_for_secondary_command_buffer_tasks() {
  for (uint32_t i = 0; i < frames[frame].secondary_command_buffers_used; i++) {
    WorkerThreadPool::TaskID& task = frames[frame].secondary_command_buffers[i].task;
    if (task != WorkerThreadPool::INVALID_TASK_ID) {
      WorkerThreadPool::get_singleton()->wait_for_task_completion(task);
      task = WorkerThreadPool::INVALID_TASK_ID;
    }
  }
}

void RenderingDeviceGraph::finalize() {
  _wait_for_secondary_command_buffer_tasks();

  for (Frame& f : frames) {
    for (SecondaryCommandBuffer& secondary : f.secondary_command_buffers) {
      if (secondary.command_pool.id != 0) {
        driver->command_pool_free(secondary.command_pool);
      }
    }
  }

  frames.clear();
}

RenderingDeviceGraph::ResourceTracker* RenderingDeviceGraph::resource_tracker_create() {
#if PRINT_RESOURCE_TRACKER_TOTAL
  print_line("Resource trackers:", ++resource_tracker_total);
#endif
  return memnew(ResourceTracker);
}

void RenderingDeviceGraph::resource_tracker_free(ResourceTracker* tracker) {
  if (tracker == nullptr) {
    return;
  }

  if (tracker->in_parent_dirty_list) {
    // Delete the tracker from the parent's dirty linked list.
    if (tracker->parent->dirty_shared_list == tracker) {
      tracker->parent->dirty_shared_list = tracker->next_shared;
    } else {
      ResourceTracker* node = tracker->parent->dirty_shared_list;
      while (node != nullptr) {
        if (node->next_shared == tracker) {
          node->next_shared = tracker->next_shared;
          node = nullptr;
        } else {
          node = node->next_shared;
        }
      }
    }
  }

  memdelete(tracker);

#if PRINT_RESOURCE_TRACKER_TOTAL
  print_line("Resource trackers:", --resource_tracker_total);
#endif
}

void RenderingDeviceGraph::_print_render_commands(const RecordedCommandSort* p_sorted_commands,
                                                  uint32_t p_sorted_commands_count) {
  for (uint32_t i = 0; i < p_sorted_commands_count; i++) {
    const uint32_t command_index = p_sorted_commands[i].index;
    const uint32_t command_level = p_sorted_commands[i].level;
    const uint32_t command_data_offset = command_data_offsets[command_index];
    const RecordedCommand* command =
        reinterpret_cast<RecordedCommand*>(&command_data[command_data_offset]);
    switch (command->type) {
      case RecordedCommand::TYPE_BUFFER_CLEAR: {
        const RecordedBufferClearCommand* buffer_clear_command =
            reinterpret_cast<const RecordedBufferClearCommand*>(command);
        print_line(command_index, "LEVEL", command_level, "BUFFER CLEAR DESTINATION",
                   itos(buffer_clear_command->buffer.id));
      } break;
      case RecordedCommand::TYPE_BUFFER_COPY: {
        const RecordedBufferCopyCommand* buffer_copy_command =
            reinterpret_cast<const RecordedBufferCopyCommand*>(command);
        print_line(command_index, "LEVEL", command_level, "BUFFER COPY SOURCE",
                   itos(buffer_copy_command->source.id), "DESTINATION",
                   itos(buffer_copy_command->destination.id));
      } break;
      case RecordedCommand::TYPE_BUFFER_GET_DATA: {
        const RecordedBufferGetDataCommand* buffer_get_data_command =
            reinterpret_cast<const RecordedBufferGetDataCommand*>(command);
        print_line(command_index, "LEVEL", command_level, "BUFFER GET DATA DESTINATION",
                   itos(buffer_get_data_command->destination.id));
      } break;
      case RecordedCommand::TYPE_BUFFER_UPDATE: {
        const RecordedBufferUpdateCommand* buffer_update_command =
            reinterpret_cast<const RecordedBufferUpdateCommand*>(command);
        print_line(command_index, "LEVEL", command_level, "BUFFER UPDATE DESTINATION",
                   itos(buffer_update_command->destination.id), "COPIES",
                   buffer_update_command->buffer_copies_count);
      } break;
      case RecordedCommand::TYPE_COMPUTE_LIST: {
        const RecordedComputeListCommand* compute_list_command =
            reinterpret_cast<const RecordedComputeListCommand*>(command);
        print_line(command_index, "LEVEL", command_level, "COMPUTE LIST SIZE",
                   compute_list_command->instruction_data_size);
      } break;
      case RecordedCommand::TYPE_DRAW_LIST: {
        const RecordedDrawListCommand* draw_list_command =
            reinterpret_cast<const RecordedDrawListCommand*>(command);
        print_line(command_index, "LEVEL", command_level, "DRAW LIST SIZE",
                   draw_list_command->instruction_data_size);
      } break;
      case RecordedCommand::TYPE_TEXTURE_CLEAR: {
        const RecordedTextureClearCommand* texture_clear_command =
            reinterpret_cast<const RecordedTextureClearCommand*>(command);
        print_line(command_index, "LEVEL", command_level, "TEXTURE CLEAR",
                   itos(texture_clear_command->texture.id), "COLOR", texture_clear_command->color);
      } break;
      case RecordedCommand::TYPE_TEXTURE_COPY: {
        const RecordedTextureCopyCommand* texture_copy_command =
            reinterpret_cast<const RecordedTextureCopyCommand*>(command);
        print_line(command_index, "LEVEL", command_level, "TEXTURE COPY FROM",
                   itos(texture_copy_command->from_texture.id), "TO",
                   itos(texture_copy_command->to_texture.id));
      } break;
      case RecordedCommand::TYPE_TEXTURE_GET_DATA: {
        print_line(command_index, "LEVEL", command_level, "TEXTURE GET DATA");
      } break;
      case RecordedCommand::TYPE_TEXTURE_RESOLVE: {
        const RecordedTextureResolveCommand* texture_resolve_command =
            reinterpret_cast<const RecordedTextureResolveCommand*>(command);
        print_line(command_index, "LEVEL", command_level, "TEXTURE RESOLVE FROM",
                   itos(texture_resolve_command->from_texture.id), "TO",
                   itos(texture_resolve_command->to_texture.id));
      } break;
      case RecordedCommand::TYPE_TEXTURE_UPDATE: {
        const RecordedTextureUpdateCommand* texture_update_command =
            reinterpret_cast<const RecordedTextureUpdateCommand*>(command);
        print_line(command_index, "LEVEL", command_level, "TEXTURE UPDATE TO",
                   itos(texture_update_command->to_texture.id));
      } break;
      case RecordedCommand::TYPE_CAPTURE_TIMESTAMP: {
        const RecordedCaptureTimestampCommand* texture_capture_timestamp_command =
            reinterpret_cast<const RecordedCaptureTimestampCommand*>(command);
        print_line(command_index, "LEVEL", command_level, "CAPTURE TIMESTAMP POOL",
                   itos(texture_capture_timestamp_command->pool.id), "INDEX",
                   texture_capture_timestamp_command->index);
      } break;
      default:
        DEV_ASSERT(false && "Unknown recorded command type.");
        return;
    }
  }
}