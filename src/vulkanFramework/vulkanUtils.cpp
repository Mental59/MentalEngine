#include "vulkanUtils.hpp"
#include "utils/utils.hpp"
#include <string>
#include <cassert>
#include <cstdlib>

namespace
{
size_t compileShader(glslang_stage_t stage, const char* shaderSource, mental::ShaderModule& shaderModule)
{
	const glslang_resource_t defaultResource = {.max_lights = 32,
		.max_clip_planes = 6,
		.max_texture_units = 32,
		.max_texture_coords = 32,
		.max_vertex_attribs = 64,
		.max_vertex_uniform_components = 4096,
		.max_varying_floats = 64,
		.max_vertex_texture_image_units = 32,
		.max_combined_texture_image_units = 80,
		.max_texture_image_units = 32,
		.max_fragment_uniform_components = 4096,
		.max_draw_buffers = 32,
		.max_vertex_uniform_vectors = 128,
		.max_varying_vectors = 8,
		.max_fragment_uniform_vectors = 16,
		.max_vertex_output_vectors = 16,
		.max_fragment_input_vectors = 15,
		.min_program_texel_offset = -8,
		.max_program_texel_offset = 7,
		.max_clip_distances = 8,
		.max_compute_work_group_count_x = 65535,
		.max_compute_work_group_count_y = 65535,
		.max_compute_work_group_count_z = 65535,
		.max_compute_work_group_size_x = 1024,
		.max_compute_work_group_size_y = 1024,
		.max_compute_work_group_size_z = 64,
		.max_compute_uniform_components = 1024,
		.max_compute_texture_image_units = 16,
		.max_compute_image_uniforms = 8,
		.max_compute_atomic_counters = 8,
		.max_compute_atomic_counter_buffers = 1,
		.max_varying_components = 60,
		.max_vertex_output_components = 64,
		.max_geometry_input_components = 64,
		.max_geometry_output_components = 128,
		.max_fragment_input_components = 128,
		.max_image_units = 8,
		.max_combined_image_units_and_fragment_outputs = 8,
		.max_combined_shader_output_resources = 8,
		.max_image_samples = 0,
		.max_vertex_image_uniforms = 0,
		.max_tess_control_image_uniforms = 0,
		.max_tess_evaluation_image_uniforms = 0,
		.max_geometry_image_uniforms = 0,
		.max_fragment_image_uniforms = 8,
		.max_combined_image_uniforms = 8,
		.max_geometry_texture_image_units = 16,
		.max_geometry_output_vertices = 256,
		.max_geometry_total_output_components = 1024,
		.max_geometry_uniform_components = 1024,
		.max_geometry_varying_components = 64,
		.max_tess_control_input_components = 128,
		.max_tess_control_output_components = 128,
		.max_tess_control_texture_image_units = 16,
		.max_tess_control_uniform_components = 1024,
		.max_tess_control_total_output_components = 4096,
		.max_tess_evaluation_input_components = 128,
		.max_tess_evaluation_output_components = 128,
		.max_tess_evaluation_texture_image_units = 16,
		.max_tess_evaluation_uniform_components = 1024,
		.max_tess_patch_components = 120,
		.max_patch_vertices = 32,
		.max_tess_gen_level = 64,
		.max_viewports = 16,
		.max_vertex_atomic_counters = 0,
		.max_tess_control_atomic_counters = 0,
		.max_tess_evaluation_atomic_counters = 0,
		.max_geometry_atomic_counters = 0,
		.max_fragment_atomic_counters = 8,
		.max_combined_atomic_counters = 8,
		.max_atomic_counter_bindings = 1,
		.max_vertex_atomic_counter_buffers = 0,
		.max_tess_control_atomic_counter_buffers = 0,
		.max_tess_evaluation_atomic_counter_buffers = 0,
		.max_geometry_atomic_counter_buffers = 0,
		.max_fragment_atomic_counter_buffers = 1,
		.max_combined_atomic_counter_buffers = 1,
		.max_atomic_counter_buffer_size = 16384,
		.max_transform_feedback_buffers = 4,
		.max_transform_feedback_interleaved_components = 64,
		.max_cull_distances = 8,
		.max_combined_clip_and_cull_distances = 8,
		.max_samples = 4,
		.max_mesh_output_vertices_nv = 256,
		.max_mesh_output_primitives_nv = 512,
		.max_mesh_work_group_size_x_nv = 32,
		.max_mesh_work_group_size_y_nv = 1,
		.max_mesh_work_group_size_z_nv = 1,
		.max_task_work_group_size_x_nv = 32,
		.max_task_work_group_size_y_nv = 1,
		.max_task_work_group_size_z_nv = 1,
		.max_mesh_view_count_nv = 4,
		.maxDualSourceDrawBuffersEXT = 1,

		.limits = {
			.non_inductive_for_loops = 1,
			.while_loops = 1,
			.do_while_loops = 1,
			.general_uniform_indexing = 1,
			.general_attribute_matrix_vector_indexing = 1,
			.general_varying_indexing = 1,
			.general_sampler_indexing = 1,
			.general_variable_indexing = 1,
			.general_constant_matrix_vector_indexing = 1,
		}};

	const glslang_input_t input = {
		.language = GLSLANG_SOURCE_GLSL,
		.stage = stage,
		.client = GLSLANG_CLIENT_VULKAN,
		.client_version = GLSLANG_TARGET_VULKAN_1_4,
		.target_language = GLSLANG_TARGET_SPV,
		.target_language_version = GLSLANG_TARGET_SPV_1_6,
		.code = shaderSource,
		.default_version = 100,
		.default_profile = GLSLANG_NO_PROFILE,
		.force_default_version_and_profile = false,
		.forward_compatible = false,
		.messages = GLSLANG_MSG_DEFAULT_BIT,
		.resource = &defaultResource,
	};

	glslang_shader_t* shader = glslang_shader_create(&input);

	if (!glslang_shader_preprocess(shader, &input))
	{
		fprintf(stderr, "GLSL preprocessing failed\n");
		fprintf(stderr, "\n%s", glslang_shader_get_info_log(shader));
		fprintf(stderr, "\n%s", glslang_shader_get_info_debug_log(shader));
		mental::printShaderSource(input.code);
		return 0;
	}

	if (!glslang_shader_parse(shader, &input))
	{
		fprintf(stderr, "GLSL parsing failed\n");
		fprintf(stderr, "\n%s", glslang_shader_get_info_log(shader));
		fprintf(stderr, "\n%s", glslang_shader_get_info_debug_log(shader));
		mental::printShaderSource(glslang_shader_get_preprocessed_code(shader));
		return 0;
	}

	glslang_program_t* program = glslang_program_create();
	glslang_program_add_shader(program, shader);

	if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
	{
		fprintf(stderr, "GLSL linking failed\n");
		fprintf(stderr, "\n%s", glslang_program_get_info_log(program));
		fprintf(stderr, "\n%s", glslang_program_get_info_debug_log(program));
		return 0;
	}

	glslang_program_SPIRV_generate(program, stage);

	shaderModule.SPIRV.resize(glslang_program_SPIRV_get_size(program));
	glslang_program_SPIRV_get(program, shaderModule.SPIRV.data());

	{
		const char* spirv_messages = glslang_program_SPIRV_get_messages(program);

		if (spirv_messages)
		{
			fprintf(stderr, "%s", spirv_messages);
		}
	}

	glslang_program_delete(program);
	glslang_shader_delete(shader);

	return shaderModule.SPIRV.size();
}
}  // namespace

glslang_stage_t mental::glslangShaderStageFromFileName(const char* fileName)
{
	if (endsWith(fileName, ".vert")) return GLSLANG_STAGE_VERTEX;

	if (endsWith(fileName, ".frag")) return GLSLANG_STAGE_FRAGMENT;

	if (endsWith(fileName, ".geom")) return GLSLANG_STAGE_GEOMETRY;

	if (endsWith(fileName, ".comp")) return GLSLANG_STAGE_COMPUTE;

	if (endsWith(fileName, ".tesc")) return GLSLANG_STAGE_TESSCONTROL;

	if (endsWith(fileName, ".tese")) return GLSLANG_STAGE_TESSEVALUATION;

	return GLSLANG_STAGE_VERTEX;
}

size_t mental::compileShaderFile(const char* file, ShaderModule& shaderModule)
{
	std::string shaderSource = readShaderFile(file);

	if (shaderSource.empty())
	{
		return 0;
	}

	return compileShader(glslangShaderStageFromFileName(file), shaderSource.c_str(), shaderModule);
}

void mental::check(bool check, const char* fileName, int lineNumber)
{
	if (!check)
	{
		printf("CHECK() failed at %s:%i\n", fileName, lineNumber);
		assert(false);
		exit(EXIT_FAILURE);
	}
}
