#include "rendering_device_graph.h"
using namespace lain::graphics;
// 主要分为read和write两种
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
  // 根据resource tracker
  for (uint32_t i = 0; i < p_resource_count; i++) {
    ResourceTracker* resource_tracker = p_resource_trackers[i];
    DEV_ASSERT(resource_tracker != nullptr);
    Rect2i resource_tracker_rect;
    resource_tracker->reset_if_outdated(tracking_frame);
    if (resource_tracker->is_texture()) {
      const RDD::TextureSubresourceRange& subresources = resource_tracker->texture_subresources;
      resource_tracker_rect = Rect2i(subresources.base_mipmap, subresources.base_layer,
                                     subresources.mipmap_count, subresources.layer_count);
    } else if (resource_tracker->is_buffer()) {
      const RDD::BufferRange& buffer_range = resource_tracker->buffer_range;
      resource_tracker_rect = Rect2i(buffer_range.offset, 0, buffer_range.size, 0);
    } else {
      DEV_ASSERT(false && "Resource tracker does not contain a valid buffer or texture ID.");
    }

    Rect2i search_tracker_rect = resource_tracker_rect;

    ResourceUsage new_resource_usage = p_resource_usages[i];
    bool write_usage = _is_write_usage(new_resource_usage);
    BitField<RDD::BarrierAccessBits> new_usage_access = _usage_to_access_bits(new_resource_usage);
    bool is_resource_a_slice = resource_tracker->parent != nullptr;
    if (is_resource_a_slice) {
      // This resource depends on a parent resource.
    } else {
    }
    // 如果只是一个切片就用父亲的
    bool resource_has_parent = resource_tracker->parent != nullptr;
    ResourceTracker* search_tracker =
        resource_has_parent ? resource_tracker->parent : resource_tracker;

    bool diff_usage = resource_tracker->usage != new_resource_usage;
    bool write_usage_after_write = (write_usage && search_tracker->write_cmd_idx >= 0);

    if (diff_usage || write_usage_after_write) {
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
      } else { 
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
    if (_usage_to_image_layout(new_resource_usage) !=
        _usage_to_image_layout(resource_tracker->usage)) {
      write_usage = true;
      resource_tracker->usage = new_resource_usage;
    }
    if (search_tracker->has_write()) {
      // Make this command adjacent to any commands that wrote to this resource and intersect with the slice if it applies.
      // For buffers or textures that never use slices, this list will only be one element long at most.
      int32_t write_cmd_idx = search_tracker->write_cmd_idx; // 能保证write_cmd_idx都是属于该资源的
      int32_t previous_write_cmd_idx = -1;
      while (write_cmd_idx >= 0) {
		// 查找所有的write，如果有交集就添加依赖 
        const RecordedSliceListNode& write_list_node = write_slice_list_nodes[write_cmd_idx];
        if (!resource_has_parent || search_tracker_rect.intersects(write_list_node.subresources)) {
          if (write_list_node.command_index == p_command_index) {
            ERR_FAIL_COND_MSG(!resource_has_parent, "Command can't have itself as a dependency.");
          } else {
            // Command is dependent on this command. Add this command to the adjacency list of the write command.
            _add_adjacent_command(write_list_node.command_index, p_command_index, r_command);

            if (resource_has_parent && write_usage &&
                search_tracker_rect.encloses(write_list_node.subresources)) {
              // Eliminate redundant writes from the list.
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
    }
  }
}
#if USE_BUFFER_BARRIERS
void RenderingDeviceGraph::_add_buffer_barrier_to_command(
    RDD::BufferID p_buffer_id, BitField<RDD::BarrierAccessBits> p_src_access,
    BitField<RDD::BarrierAccessBits> p_dst_access, RDD::BufferRange p_range,
    int32_t& r_barrier_index, int32_t& r_barrier_count) {
  // if (!driver_honors_barriers) {
  // 	return;
  // }

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
  // if (!driver_honors_barriers) {
  // 	return;
  // }

  if (r_barrier_index < 0) {
    r_barrier_index = r_barrier_vector.size();
  }

  RDD::TextureBarrier texture_barrier;
  texture_barrier.texture = p_texture_id;
  texture_barrier.src_access = p_src_access;
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