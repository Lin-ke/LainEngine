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

RDD::BarrierAccessBits RenderingDeviceGraph::_usage_to_access_bits(ResourceUsage p_usage) {
#if FORCE_FULL_ACCESS_BITS
	return RDD::BarrierAccessBits(RDD::BARRIER_ACCESS_MEMORY_READ_BIT | RDD::BARRIER_ACCESS_MEMORY_WRITE_BIT);
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
			return RDD::BarrierAccessBits(RDD::BARRIER_ACCESS_SHADER_READ_BIT | RDD::BARRIER_ACCESS_SHADER_WRITE_BIT);
		case RESOURCE_USAGE_VERTEX_BUFFER_READ:
			return RDD::BARRIER_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
		case RESOURCE_USAGE_INDEX_BUFFER_READ:
			return RDD::BARRIER_ACCESS_INDEX_READ_BIT;
		case RESOURCE_USAGE_ATTACHMENT_COLOR_READ_WRITE:
			return RDD::BarrierAccessBits(RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_READ_BIT | RDD::BARRIER_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
		case RESOURCE_USAGE_ATTACHMENT_DEPTH_STENCIL_READ_WRITE:
			return RDD::BarrierAccessBits(RDD::BARRIER_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | RDD::BARRIER_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
		default:
			DEV_ASSERT(false && "Invalid usage.");
			return RDD::BarrierAccessBits(0);
	}
#endif
}

void RDG::_add_command_to_graph(ResourceTracker **p_resource_trackers, ResourceUsage *p_resource_usages, uint32_t p_resource_count, int32_t p_command_index, RecordedCommand *r_command) {
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
        ResourceTracker *resource_tracker = p_resource_trackers[i];
		DEV_ASSERT(resource_tracker != nullptr);

		resource_tracker->reset_if_outdated(tracking_frame);

        const RDD::TextureSubresourceRange &subresources = resource_tracker->texture_subresources;
		const Rect2i resource_tracker_rect(subresources.base_mipmap, subresources.base_layer, subresources.mipmap_count, subresources.layer_count);
		Rect2i search_tracker_rect = resource_tracker_rect;

		ResourceUsage new_resource_usage = p_resource_usages[i];
		bool write_usage = _is_write_usage(new_resource_usage);
		BitField<RDD::BarrierAccessBits> new_usage_access = _usage_to_access_bits(new_resource_usage);
		bool is_resource_a_slice = resource_tracker->parent != nullptr;
        if(is_resource_a_slice) {
        }
        else{

        }
        // search operation
    }


}

void RenderingDeviceGraph::_add_adjacent_command(int32_t p_previous_command_index, int32_t p_command_index, RecordedCommand *r_command) {
	const uint32_t previous_command_data_offset = command_data_offsets[p_previous_command_index];
	RecordedCommand &previous_command = *reinterpret_cast<RecordedCommand *>(&command_data[previous_command_data_offset]);
	previous_command.adjacent_command_list_index = _add_to_command_list(p_command_index, previous_command.adjacent_command_list_index); // 倒着插入
	previous_command.next_stages = previous_command.next_stages | r_command->self_stages;
	r_command->previous_stages = r_command->previous_stages | previous_command.self_stages;
}

int32_t RenderingDeviceGraph::_add_to_command_list(int32_t p_command_index, int32_t p_list_index) {
	DEV_ASSERT(p_command_index < int32_t(command_count));
	DEV_ASSERT(p_list_index < int32_t(command_list_nodes.size()));

	int32_t next_index = int32_t(command_list_nodes.size());
	command_list_nodes.resize(next_index + 1);

	RecordedCommandListNode &new_node = command_list_nodes[next_index];
	new_node.command_index = p_command_index;
	new_node.next_list_index = p_list_index;
	return next_index;
}