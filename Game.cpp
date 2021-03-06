#include "Game.hpp"

#include "gl_errors.hpp" //helper for dumpping OpenGL error messages
#include "read_chunk.hpp" //helper for reading a vector of structures from a file
#include "data_path.hpp" //helper to get paths relative to executable
#include "utils.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <iostream>
#include <fstream>
#include <map>
#include <cstddef>
#include <random>

//helper defined later; throws if shader compilation fails:
static GLuint compile_shader(GLenum type, std::string const &source);

Game::Game() {
	{ //create an opengl program to perform sun/sky (well, directional+hemispherical) lighting:
		GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER,
			"#version 330\n"
			"uniform mat4 object_to_clip;\n"
			"uniform mat4x3 object_to_light;\n"
			"uniform mat3 normal_to_light;\n"
			"layout(location=0) in vec4 Position;\n" //note: layout keyword used to make sure that the location-0 attribute is always bound to something
			"in vec3 Normal;\n"
			"in vec4 Color;\n"
			"out vec3 position;\n"
			"out vec3 normal;\n"
			"out vec4 color;\n"
			"void main() {\n"
			"	gl_Position = object_to_clip * Position;\n"
			"	position = object_to_light * Position;\n"
			"	normal = normal_to_light * Normal;\n"
			"	color = Color;\n"
			"}\n"
		);

		GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER,
			"#version 330\n"
			"uniform vec3 sun_direction;\n"
			"uniform vec3 sun_color;\n"
			"uniform vec3 sky_direction;\n"
			"uniform vec3 sky_color;\n"
			"in vec3 position;\n"
			"in vec3 normal;\n"
			"in vec4 color;\n"
			"out vec4 fragColor;\n"
			"void main() {\n"
			"	vec3 total_light = vec3(0.0, 0.0, 0.0);\n"
			"	vec3 n = normalize(normal);\n"
			"	{ //sky (hemisphere) light:\n"
			"		vec3 l = sky_direction;\n"
			"		float nl = 0.5 + 0.5 * dot(n,l);\n"
			"		total_light += nl * sky_color;\n"
			"	}\n"
			"	{ //sun (directional) light:\n"
			"		vec3 l = sun_direction;\n"
			"		float nl = max(0.0, dot(n,l));\n"
			"		total_light += nl * sun_color;\n"
			"	}\n"
			"	fragColor = vec4(color.rgb * total_light, color.a);\n"
			"}\n"
		);

		simple_shading.program = glCreateProgram();
		glAttachShader(simple_shading.program, vertex_shader);
		glAttachShader(simple_shading.program, fragment_shader);
		//shaders are reference counted so this makes sure they are freed after program is deleted:
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		//link the shader program and throw errors if linking fails:
		glLinkProgram(simple_shading.program);
		GLint link_status = GL_FALSE;
		glGetProgramiv(simple_shading.program, GL_LINK_STATUS, &link_status);
		if (link_status != GL_TRUE) {
			std::cerr << "Failed to link shader program." << std::endl;
			GLint info_log_length = 0;
			glGetProgramiv(simple_shading.program, GL_INFO_LOG_LENGTH, &info_log_length);
			std::vector< GLchar > info_log(info_log_length, 0);
			GLsizei length = 0;
			glGetProgramInfoLog(simple_shading.program, GLsizei(info_log.size()), &length, &info_log[0]);
			std::cerr << "Info log: " << std::string(info_log.begin(), info_log.begin() + length);
			throw std::runtime_error("failed to link program");
		}
	}

	{ //read back uniform and attribute locations from the shader program:
		simple_shading.object_to_clip_mat4 = glGetUniformLocation(simple_shading.program, "object_to_clip");
		simple_shading.object_to_light_mat4x3 = glGetUniformLocation(simple_shading.program, "object_to_light");
		simple_shading.normal_to_light_mat3 = glGetUniformLocation(simple_shading.program, "normal_to_light");

		simple_shading.sun_direction_vec3 = glGetUniformLocation(simple_shading.program, "sun_direction");
		simple_shading.sun_color_vec3 = glGetUniformLocation(simple_shading.program, "sun_color");
		simple_shading.sky_direction_vec3 = glGetUniformLocation(simple_shading.program, "sky_direction");
		simple_shading.sky_color_vec3 = glGetUniformLocation(simple_shading.program, "sky_color");

		simple_shading.Position_vec4 = glGetAttribLocation(simple_shading.program, "Position");
		simple_shading.Normal_vec3 = glGetAttribLocation(simple_shading.program, "Normal");
		simple_shading.Color_vec4 = glGetAttribLocation(simple_shading.program, "Color");
	}

	struct Vertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::u8vec4 Color;
	};
	static_assert(sizeof(Vertex) == 28, "Vertex should be packed.");

	{ //load mesh data from a binary blob:
		std::ifstream blob(data_path("board.blob"), std::ios::binary);
		//The blob will be made up of three chunks:
		// the first chunk will be vertex data (interleaved position/normal/color)
		// the second chunk will be characters
		// the third chunk will be an index, mapping a name (range of characters) to a mesh (range of vertex data)

		//read vertex data:
		std::vector< Vertex > vertices;
		read_chunk(blob, "dat0", &vertices);

		//read character data (for names):
		std::vector< char > names;
		read_chunk(blob, "str0", &names);

		//read index:
		struct IndexEntry {
			uint32_t name_begin;
			uint32_t name_end;
			uint32_t vertex_begin;
			uint32_t vertex_end;
		};
		static_assert(sizeof(IndexEntry) == 16, "IndexEntry should be packed.");

		std::vector< IndexEntry > index_entries;
		read_chunk(blob, "idx0", &index_entries);

		if (blob.peek() != EOF) {
			std::cerr << "WARNING: trailing data in meshes file." << std::endl;
		}

		//upload vertex data to the graphics card:
		glGenBuffers(1, &meshes_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, meshes_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//create map to store index entries:
		std::map< std::string, Mesh > index;
		for (IndexEntry const &e : index_entries) {
			if (e.name_begin > e.name_end || e.name_end > names.size()) {
				throw std::runtime_error("invalid name indices in index.");
			}
			if (e.vertex_begin > e.vertex_end || e.vertex_end > vertices.size()) {
				throw std::runtime_error("invalid vertex indices in index.");
			}
			Mesh mesh;
			mesh.first = e.vertex_begin;
			mesh.count = e.vertex_end - e.vertex_begin;
			auto ret = index.insert(std::make_pair(
				std::string(names.begin() + e.name_begin, names.begin() + e.name_end),
				mesh));
			if (!ret.second) {
				throw std::runtime_error("duplicate name in index.");
			}
		}

		//look up into index map to extract meshes:
		auto lookup = [&index](std::string const &name) -> Mesh {
			auto f = index.find(name);
			if (f == index.end()) {
				throw std::runtime_error("Mesh named '" + name + "' does not appear in index.");
			}
			return f->second;
		};
		board_mesh= lookup("Board");
		bigboss_mesh = lookup("BigBoss");
		enemy_mesh = lookup("Enemy");
		cone_mesh = lookup("Cone");
		cone_red_mesh = lookup("ConeRed");
		camera_cone_mesh = lookup("CameraCone");
		camera_cone_red_mesh = lookup("CameraConeRed");
		box_mesh = lookup("Box");
		check_point_mesh = lookup("CheckPoint");
		destination_mesh = lookup("Destination");
		game_over_mesh = lookup("Game Over");
		level_clear_mesh = lookup("Level Clear");
}

	{ //create vertex array object to hold the map from the mesh vertex buffer to shader program attributes:
		glGenVertexArrays(1, &meshes_for_simple_shading_vao);
		glBindVertexArray(meshes_for_simple_shading_vao);
		glBindBuffer(GL_ARRAY_BUFFER, meshes_vbo);
		//note that I'm specifying a 3-vector for a 4-vector attribute here, and this is okay to do:
		glVertexAttribPointer(simple_shading.Position_vec4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLbyte *)0 + offsetof(Vertex, Position));
		glEnableVertexAttribArray(simple_shading.Position_vec4);
		if (simple_shading.Normal_vec3 != -1U) {
			glVertexAttribPointer(simple_shading.Normal_vec3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLbyte *)0 + offsetof(Vertex, Normal));
			glEnableVertexAttribArray(simple_shading.Normal_vec3);
		}
		if (simple_shading.Color_vec4 != -1U) {
			glVertexAttribPointer(simple_shading.Color_vec4, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLbyte *)0 + offsetof(Vertex, Color));
			glEnableVertexAttribArray(simple_shading.Color_vec4);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	GL_ERRORS();
}

Game::~Game() {
	glDeleteVertexArrays(1, &meshes_for_simple_shading_vao);
	meshes_for_simple_shading_vao = -1U;

	glDeleteBuffers(1, &meshes_vbo);
	meshes_vbo = -1U;

	glDeleteProgram(simple_shading.program);
	simple_shading.program = -1U;

	GL_ERRORS();
}

bool Game::handle_event(SDL_Event const &evt, glm::uvec2 window_size) {
	//ignore any keys that are the result of automatic key repeat:
	if (evt.type == SDL_KEYDOWN && evt.key.repeat) {
		return false;
	}
	//handle tracking the state of WSAD for roll control:
	if (evt.type == SDL_KEYDOWN || evt.type == SDL_KEYUP) {
		if (evt.key.keysym.scancode == SDL_SCANCODE_UP) {
		    big_boss.is_box = false;
			controls.roll_up = (evt.type == SDL_KEYDOWN);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_DOWN) {
            big_boss.is_box = false;
			controls.roll_down = (evt.type == SDL_KEYDOWN);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_LEFT) {
            big_boss.is_box = false;
			controls.roll_left = (evt.type == SDL_KEYDOWN);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
            big_boss.is_box = false;
			controls.roll_right = (evt.type == SDL_KEYDOWN);
			return true;
		}
	}

	if (evt.type == SDL_KEYDOWN && evt.key.repeat == 0) {
	    if (evt.key.keysym.scancode == SDL_SCANCODE_SPACE) {
	        board.angle_horizontal = 0.0f;
	        board.angle_vertical = 0.0f;
	        board_rotate = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

	        big_boss.velocity = glm::vec2(0.0f, 0.0f);
	        big_boss.is_box = true;
	        return true;
	    }

	    if (evt.key.keysym.scancode == SDL_SCANCODE_RETURN && (board.level_clear || game_over)) {
	        board.level_clear = false;
	        game_over = false;

	        board.angle_horizontal = 0.0f;
	        board.angle_vertical = 0.0f;
	        board_rotate = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

	        big_boss.velocity = glm::vec2(0.0f, 0.0f);
	        big_boss.is_box = false;

	        big_boss.position.x = -14;
	        big_boss.position.y = 14;

	        board.checkpoint_counter = 0;

	        board.init_map();
	        return true;
	    }
	}
	return false;
}

void Game::update(float elapsed) {
	//if the roll keys are pressed, rotate everything on the same row or column as the cursor:
	glm::quat dr = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	float amt = elapsed * 1.0f;

	if (game_over) return;
	if (board.level_clear) return;

	if (controls.roll_left) {
	    if (board.angle_horizontal > -BOARD_ROTATE_ANGLE_HORIZONTAL_LIMIT) {
	        float da = amt;

	        if (board.angle_horizontal - amt <= -BOARD_ROTATE_ANGLE_HORIZONTAL_LIMIT) {
	            da = board.angle_horizontal + BOARD_ROTATE_ANGLE_HORIZONTAL_LIMIT;
	            board.angle_horizontal = -BOARD_ROTATE_ANGLE_HORIZONTAL_LIMIT;
	        } else {
	            board.angle_horizontal -= amt;
	        }

            dr = glm::angleAxis(-da, board_rotate * glm::vec3(0.0f, 1.0f, 0.0f)) * dr;
        } else {
	        board.angle_horizontal = -BOARD_ROTATE_ANGLE_HORIZONTAL_LIMIT;
	    }
	}
	if (controls.roll_right) {
	    if (board.angle_horizontal < BOARD_ROTATE_ANGLE_HORIZONTAL_LIMIT) {
	        float da = amt;

	        if (board.angle_horizontal + amt >= BOARD_ROTATE_ANGLE_HORIZONTAL_LIMIT) {
	            da = BOARD_ROTATE_ANGLE_HORIZONTAL_LIMIT - board.angle_horizontal;
	            board.angle_horizontal = BOARD_ROTATE_ANGLE_HORIZONTAL_LIMIT;
	        } else {
	            board.angle_horizontal += amt;
	        }

            dr = glm::angleAxis(da, board_rotate * glm::vec3(0.0f, 1.0f, 0.0f)) * dr;
	    } else {
	        board.angle_horizontal = BOARD_ROTATE_ANGLE_HORIZONTAL_LIMIT;
	    }
	}
	if (controls.roll_up) {
	    if (board.angle_vertical > -BOARD_ROTATE_ANGLE_VERTICAL_LIMIT) {

	         float da = amt;

	        if (board.angle_vertical - amt <= -BOARD_ROTATE_ANGLE_VERTICAL_LIMIT) {
	            da = board.angle_vertical + BOARD_ROTATE_ANGLE_VERTICAL_LIMIT;
	            board.angle_vertical= -BOARD_ROTATE_ANGLE_VERTICAL_LIMIT;
	        } else {
	            board.angle_vertical -= amt;
	        }

            dr = glm::angleAxis(-da, board_rotate * glm::vec3(1.0f, 0.0f, 0.0f)) * dr;
        } else {
            board.angle_vertical = -BOARD_ROTATE_ANGLE_VERTICAL_LIMIT;
	    }
	}

	if (controls.roll_down) {
	    if (board.angle_vertical < BOARD_ROTATE_ANGLE_VERTICAL_LIMIT) {
	        float da = amt;

	        if (board.angle_vertical+ amt >= BOARD_ROTATE_ANGLE_VERTICAL_LIMIT) {
	            da = BOARD_ROTATE_ANGLE_VERTICAL_LIMIT - board.angle_vertical;
	            board.angle_vertical = BOARD_ROTATE_ANGLE_VERTICAL_LIMIT;
	        } else {
	            board.angle_vertical += amt;
	        }

            dr = glm::angleAxis(da, board_rotate * glm::vec3(1.0f, 0.0f, 0.0f)) * dr;
        } else {
            board.angle_vertical = BOARD_ROTATE_ANGLE_VERTICAL_LIMIT;
	    }
	}

	if (dr != glm::quat()) {
	    board_rotate = glm::normalize(dr * board_rotate);
	}

	big_boss.update(elapsed, board);
	for(auto &enemy : enemy_array) {
	    enemy.update(elapsed);
	    if (enemy.intercept_with(big_boss) && !big_boss.is_box) {
	        game_over = true;
	        std::cerr << "GAME OVER!" << std::endl;
	    }
	}

	for (auto &security_camera: security_camera_array) {
	    security_camera.update(elapsed);
	    if (security_camera.intercept_with(big_boss) && !big_boss.is_box) {
	    	game_over = true;
			std::cerr << "GAME OVER!" << std::endl;
	    }
	}
}

void Game::draw(glm::uvec2 drawable_size) {
	//Set up a transformation matrix to fit the board in the window:
	glm::mat4 world_to_clip, world_to_clip_ortho;
	{
		float aspect = float(drawable_size.x) / float(drawable_size.y);

		world_to_clip = glm::perspective(
			glm::radians(90.0f),
			aspect,
			0.1f,
			500.0f
		)
		 * glm::lookAt(
			glm::vec3(0.0f, -10.0f, 25.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);

		world_to_clip_ortho = glm::perspective(
			glm::radians(90.0f),
			aspect,
			0.1f,
			500.0f
		)
		 * glm::lookAt(
			glm::vec3(0.0f, 0.0f, 25.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
	}

	//set up graphics pipeline to use data from the meshes and the simple shading program:
	glBindVertexArray(meshes_for_simple_shading_vao);
	glUseProgram(simple_shading.program);

	glUniform3fv(simple_shading.sun_color_vec3, 1, glm::value_ptr(glm::vec3(0.81f, 0.76f, 0.76f)));
	glUniform3fv(simple_shading.sun_direction_vec3, 1, glm::value_ptr(glm::normalize(glm::vec3(-0.2f, 0.2f, 1.0f))));
	glUniform3fv(simple_shading.sky_color_vec3, 1, glm::value_ptr(glm::vec3(0.2f, 0.2f, 0.3f)));
	glUniform3fv(simple_shading.sky_direction_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 1.0f, 0.0f)));

	//helper function to draw a given mesh with a given transformation:
	auto draw_mesh = [&](Mesh const &mesh, glm::mat4 const &object_to_world, bool ortho = false) {
		//set up the matrix uniforms:
		if (simple_shading.object_to_clip_mat4 != -1U) {
			glm::mat4 object_to_clip;
		    if (!ortho)
				object_to_clip = world_to_clip * object_to_world;
			else
				object_to_clip = world_to_clip_ortho * object_to_world;
			glUniformMatrix4fv(simple_shading.object_to_clip_mat4, 1, GL_FALSE, glm::value_ptr(object_to_clip));
		}
		if (simple_shading.object_to_light_mat4x3 != -1U) {
			glUniformMatrix4x3fv(simple_shading.object_to_light_mat4x3, 1, GL_FALSE, glm::value_ptr(object_to_world));
		}
		if (simple_shading.normal_to_light_mat3 != -1U) {
			//NOTE: if there isn't any non-uniform scaling in the object_to_world matrix, then the inverse transpose is the matrix itself, and computing it wastes some CPU time:
			glm::mat3 normal_to_world = glm::inverse(glm::transpose(glm::mat3(object_to_world)));
			glUniformMatrix3fv(simple_shading.normal_to_light_mat3, 1, GL_FALSE, glm::value_ptr(normal_to_world));
		}

		//draw the mesh:
		glDrawArrays(GL_TRIANGLES, mesh.first, mesh.count);
	};

	for (auto enemy : enemy_array) {
	    draw_mesh(enemy_mesh,
	            enemy.get_view_matrix(board)* glm::mat4_cast(board_rotate)
        );

	    if (enemy.spot && !big_boss.is_box) {
			draw_mesh(cone_red_mesh,
					  enemy.get_cone_matrix(board)
					  );
	    } else {
			draw_mesh(cone_mesh,
					  enemy.get_cone_matrix(board)
			);
		}
	}

	for (auto security_camera: security_camera_array) {
	    if (security_camera.spot && !big_boss.is_box) {
			draw_mesh(camera_cone_red_mesh,
					  security_camera.get_cone_matrix(board)
			);
	    } else {
			draw_mesh(camera_cone_mesh,
					  security_camera.get_cone_matrix(board)
			);
		}
	}


	// Draw BigBoss
	if (big_boss.is_box){
	    draw_mesh(box_mesh,
                  big_boss.get_view_matrix(board.angle_horizontal, board.angle_vertical)* glm::mat4_cast(board_rotate)
        );
	} else {
        draw_mesh(bigboss_mesh,
                  big_boss.get_view_matrix(board.angle_horizontal, board.angle_vertical)* glm::mat4_cast(board_rotate)
        );
	}


	// Draw the board frame
	draw_mesh(board_mesh,
              glm::mat4(
                      1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f
              ) * glm::mat4_cast(board_rotate)
    );

	// Draw the board walls

	for (int y = 0; y < 30; y++) {
		for (int x = 0; x < 30; x++) {
			if(board.map[y][x] == 2) {
			    float coordinate_x = x - 14.5f;
			    float coordinate_y = 14.5f - y;

			    draw_mesh(check_point_mesh,
			            glm::mat4(
			            		1.0f, 0.0f, 0.0f, 0.0f,
			            		0.0f, 1.0f, 0.0f, 0.0f,
			            		0.0f, 0.0f, 1.0f, 0.0f,
			            		coordinate_x, coordinate_y, get_height(board.angle_horizontal, board.angle_vertical, glm::vec2(coordinate_x, coordinate_y)), 1.0f
						) * glm::mat4_cast(board_rotate)
				);
			}

			if (board.map[y][x] == 5) {
				float coordinate_x = x - 14.5f;
			    float coordinate_y = 14.5f - y;

			    draw_mesh(destination_mesh,
			            glm::mat4(
			            		1.0f, 0.0f, 0.0f, 0.0f,
			            		0.0f, 1.0f, 0.0f, 0.0f,
			            		0.0f, 0.0f, 1.0f, 0.0f,
			            		coordinate_x, coordinate_y, get_height(board.angle_horizontal, board.angle_vertical, glm::vec2(coordinate_x, coordinate_y)), 1.0f
						) * glm::mat4_cast(board_rotate)
				);
			}
		}
	}

	if (game_over) {
		draw_mesh(game_over_mesh,
				glm::mat4(
				        1.0f, 0.0f, 0.0f, 0.0f,
				        0.0f, 1.0f, 0.0f,0.0f,
				        0.0f, 0.0f, 1.0f,0.0f,
				        -8.0f, 0.0f, 15.0f, 1.0f
				), true
		);
	}

	if (board.level_clear) {
	    draw_mesh(level_clear_mesh,
				glm::mat4(
				        1.0f, 0.0f, 0.0f, 0.0f,
				        0.0f, 1.0f, 0.0f,0.0f,
				        0.0f, 0.0f, 1.0f,0.0f,
				        -8.0f, 0.0f, 15.0f, 1.0f
				), true
		);
	}

	glUseProgram(0);

	GL_ERRORS();
}



//create and return an OpenGL vertex shader from source:
static GLuint compile_shader(GLenum type, std::string const &source) {
	GLuint shader = glCreateShader(type);
	GLchar const *str = source.c_str();
	GLint length = GLint(source.size());
	glShaderSource(shader, 1, &str, &length);
	glCompileShader(shader);
	GLint compile_status = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status != GL_TRUE) {
		std::cerr << "Failed to compile shader." << std::endl;
		GLint info_log_length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector< GLchar > info_log(info_log_length, 0);
		GLsizei length = 0;
		glGetShaderInfoLog(shader, GLsizei(info_log.size()), &length, &info_log[0]);
		std::cerr << "Info log: " << std::string(info_log.begin(), info_log.begin() + length);
		glDeleteShader(shader);
		throw std::runtime_error("Failed to compile shader.");
	}
	return shader;
}
