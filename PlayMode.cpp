#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <time.h>
#include <iostream>

//for generating assets
#include "generate_asset.cpp"

PlayMode::PlayMode() {
	generate_sprites(ppu);
	//sprite 0 is spider_alive
	//sprite 1 is spider_dead
	//sprites 10-19 are clouds
	//sprites 20-29 are raindrops
	player_at.x = 16;
	player_at.y = 16;

	ppu.sprites[0].x = player_at.x;
	ppu.sprites[0].y = player_at.y;

	//randomize initial x position of clouds
	//randomize cloud directions
	//randomize cloud speeds - 20.0f, 30.0f, or 40.0f
	srand (time(NULL));
	for (int i=10;i<20;i++){
		uint8_t cloud_x = rand() % 240;
		ppu.sprites[i].x = cloud_x;
		clouds_at.push_back(glm::vec2(ppu.sprites[i].x, ppu.sprites[i].y));

		uint8_t cloud_direction = rand() % 10 + 1;
		if (cloud_direction > 5)
			clouds_direction.push_back(LEFT);
		else
			clouds_direction.push_back(RIGHT);

		uint8_t cloud_speed_classifier = rand() % 3;
		if (cloud_speed_classifier == 0)
			clouds_speed.push_back(20.0f);
		else if (cloud_speed_classifier == 1)
			clouds_speed.push_back(30.0f);
		else if (cloud_speed_classifier == 2)
			clouds_speed.push_back(40.0f);
	}

	//initialize raindrop positions
	for (int i=20;i<=29;i++){
		raindrops_at.push_back(glm::vec2(ppu.sprites[i].x, ppu.sprites[i].y));
		raindrops_falling.push_back(false);
	}
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update_clouds(float elapsed) {
	for (int i=0;i<clouds_at.size();i++){
		if (clouds_direction[i] == LEFT){
			clouds_at[i].x -= clouds_speed[i] * elapsed;
		}
		else if (clouds_direction[i] == RIGHT){
			clouds_at[i].x += clouds_speed[i] * elapsed;
		}

		//decide whether or not to drop a raindrop from the cloud
		uint8_t drop = rand() % 10 + 1;
		if (!raindrops_falling[i] && drop > 7){
			raindrops_falling[i] = true;
			raindrops_at[i].x = clouds_at[i].x;
			raindrops_at[i].y = clouds_at[i].y - 8;
		}

		//handle wrapping because it causes collision issues
		while (clouds_at[i].x < 0){
			clouds_at[i].x += ppu.ScreenWidth;
		}
		while (clouds_at[i].x >= ppu.ScreenWidth){
			clouds_at[i].x -= ppu.ScreenWidth;
		}
	}

	for (int i=10;i<20;i++){
		ppu.sprites[i].x = clouds_at[i-10].x;
	}
}

void PlayMode::update_raindrops(float elapsed){
	constexpr float FallingSpeed = 50.0f; //raindrops are fast!

	for (int i=0;i<raindrops_at.size();i++){
		if (raindrops_falling[i]){
			raindrops_at[i].y -= FallingSpeed * elapsed;
		}
		if (raindrops_at[i].y < 16){
			raindrops_falling[i] = false;
			raindrops_at[i].y = 240;
		}
		if (raindrops_at[i].y <= player_at.y + player_height){//check if the raindrop is at the right height
			if (player_at.x <= raindrops_at[i].x + raindrop_width && raindrops_at[i].x <= player_at.x + player_width){//if their x ranges overlap
				player_alive = false;
				ppu.sprites[1].x = player_at.x;
				ppu.sprites[1].y = player_at.y;
				ppu.sprites[0].y = 240;
			}
		}

		//handle wrapping because it causes collision issues
		while (raindrops_at[i].x < 0){
			raindrops_at[i].x += ppu.ScreenWidth;
		}
		while (raindrops_at[i].x >= ppu.ScreenWidth){
			raindrops_at[i].x -= ppu.ScreenWidth;
		}
	}

	for (int i=20;i<30;i++){
		ppu.sprites[i].x = raindrops_at[i-20].x;
		ppu.sprites[i].y = raindrops_at[i-20].y;
	}
}

void PlayMode::update(float elapsed) {
	constexpr float PlayerSpeed = 30.0f;
	if (left.pressed && player_alive) player_at.x -= PlayerSpeed * elapsed;
	if (right.pressed && player_alive) player_at.x += PlayerSpeed * elapsed;

	//handle wrapping because it causes collision issues
	while (player_at.x < 0){
		player_at.x += ppu.ScreenWidth;
	}
	while (player_at.x >= ppu.ScreenWidth){
		player_at.x -= ppu.ScreenWidth;
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;

	update_clouds(elapsed);
	update_raindrops(elapsed);
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	//player sprite:
	ppu.sprites[0].x = int32_t(player_at.x);
	ppu.sprites[0].y = int32_t(player_at.y);
	ppu.sprites[0].index = 0;
	ppu.sprites[0].attributes = 0;

	//--- actually draw ---
	ppu.draw(drawable_size);
}
