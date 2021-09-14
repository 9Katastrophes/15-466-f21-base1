#include "PPU466.hpp"
#include "Mode.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//helper functions
	void update_clouds(float elapsed);
	void update_raindrops(float elapsed);

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//player information
	glm::vec2 player_at = glm::vec2(0.0f);
	bool player_alive = true;
	float player_height = 5.0f; //bow doesn't add to height
	float player_width = 8.0f;

	//cloud information
	std::vector< glm::vec2 > clouds_at;
	enum Direction {
		LEFT,
		RIGHT
	};
	std::vector< Direction > clouds_direction;
	std::vector< float > clouds_speed;

	//raindrop information
	std::vector< glm::vec2 > raindrops_at;
	std::vector< bool > raindrops_falling;
	float raindrop_height = 4.0f;
	float raindrop_width = 3.0f;

	//----- drawing handled by PPU466 -----

	PPU466 ppu;
};
