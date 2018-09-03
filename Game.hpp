#pragma once

#include "GL.hpp"

#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "BigBoss.hpp"
#include "Enemy.hpp"
#include "SecurityCamera.h"

#include <vector>

// The 'Game' struct holds all of the game-relevant state,
// and is called by the main loop.

struct Game {
	//Game creates OpenGL resources (i.e. vertex buffer objects) in its
	//constructor and frees them in its destructor.
	Game();
	~Game();

	//handle_event is called when new mouse or keyboard events are received:
	// (note that this might be many times per frame or never)
	//The function should return 'true' if it handled the event.
	bool handle_event(SDL_Event const &evt, glm::uvec2 window_size);

	//update is called at the start of a new frame, after events are handled:
	void update(float elapsed);

	//draw is called after update:
	void draw(glm::uvec2 drawable_size);

	//------- opengl resources -------

	//shader program that draws lit objects with vertex colors:
	struct {
		GLuint program = -1U; //program object

		//uniform locations:
		GLuint object_to_clip_mat4 = -1U;
		GLuint object_to_light_mat4x3 = -1U;
		GLuint normal_to_light_mat3 = -1U;
		GLuint sun_direction_vec3 = -1U;
		GLuint sun_color_vec3 = -1U;
		GLuint sky_direction_vec3 = -1U;
		GLuint sky_color_vec3 = -1U;

		//attribute locations:
		GLuint Position_vec4 = -1U;
		GLuint Normal_vec3 = -1U;
		GLuint Color_vec4 = -1U;
	} simple_shading;

	//mesh data, stored in a vertex buffer:
	GLuint meshes_vbo = -1U; //vertex buffer holding mesh data

	//The location of each mesh in the meshes vertex buffer:
	struct Mesh {
		GLint first = 0;
		GLsizei count = 0;
	};

	BigBoss big_boss = BigBoss(-14, 14);
	Enemy enemy_array[3] = {
		Enemy(6.0f, 0.0f, Enemy::DIRECTION::DIRECTION_LEFT),
		Enemy(0.0f, -6.0f, Enemy::DIRECTION::DIRECTION_UP),
		Enemy(10.0f, 10.0f, Enemy::DIRECTION::DIRECTION_DOWN)
	};
	SecurityCamera security_camera_array[3] = {
	        SecurityCamera(-6.0f, 12.5f, SecurityCamera::DIRECTION::DIRECTION_DOWN),
			SecurityCamera(-12.0f, -8.0f, SecurityCamera::DIRECTION::DIRECTION_UP),
			SecurityCamera(-1.0f, -12.0f, SecurityCamera::DIRECTION::DIRECTION_RIGHT)
	};


	Mesh board_mesh;
	Mesh bigboss_mesh;
	Mesh cone_mesh;
	Mesh camera_cone_mesh;
	Mesh box_mesh;

	GLuint meshes_for_simple_shading_vao = -1U; //vertex array object that describes how to connect the meshes_vbo to the simple_shading_program

	//------- game state -------

	glm::uvec2 board_size = glm::uvec2(5,4);
	std::vector< Mesh const * > board_meshes;
	std::vector< glm::quat > board_rotations;
	glm::quat board_rotate;

	Board board;
	float board_rotate_angle_horizontal = 0.0f;
	float board_rotate_angle_vertical = 0.0f;

	const float BOARD_ROTATE_ANGLE_HORIZONTAL_LIMIT = glm::radians(5.0f);
	const float BOARD_ROTATE_ANGLE_VERTICAL_LIMIT = glm::radians(5.0f);

	glm::uvec2 cursor = glm::vec2(0,0);

	struct {
		bool roll_left = false;
		bool roll_right = false;
		bool roll_up = false;
		bool roll_down = false;
	} controls;

};
