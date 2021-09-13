#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <stdint.h>
#include <iomanip>

#include "PPU466.hpp"
#include "data_path.hpp"
#include "load_save_png.hpp"

void generate_sprites(PPU466 &ppu){
	std::string sprites_path = data_path("sprites.png");

	//we expect our sprite sheet to be 32 x 32 pixels
	//this means we have a maximum of 16 sprites
	uint32_t height = 32;
	uint32_t width = 32;
	glm::uvec2 dimensions(height, width);
	std::vector< glm::u8vec4 > sprite_sheet_pixels;

	load_png(sprites_path, &dimensions, &sprite_sheet_pixels, UpperLeftOrigin);

	//get each sprite starting from the upper left
	for (int i=0;i<16;i++){
		int sprite_upper_left = (i % 4) * 8 + 256 * (i / 4);
		std::vector< glm::u8vec4 > color_key;
		std::vector< int > color_numbering;
		
		//always include the completely transparent color
		glm::u8vec4 transparent(0, 0, 0, 0);
		color_key.push_back(transparent);

		for (int y=0;y<8;y++){
			for (int x=0;x<8;x++){
				glm::u8vec4 pixel_color = sprite_sheet_pixels[sprite_upper_left + 32 * y + x];

				bool color_found = false;
				for (int k=0;k<color_key.size();k++){
					if (color_key[k] == pixel_color){
						color_numbering.push_back(k);
						color_found = true;
						break;
					}
				}
				if (!color_found){
					color_key.push_back(pixel_color);
					color_numbering.push_back(color_key.size() - 1);
					if (color_key.size() > 4){
						std::cerr << "Tile " << i << " has too many colors!\n";
						for (int c=0;c<color_key.size();c++){
							glm::u8vec4 cc = color_key[c];
							std::cerr << "Color: " << std::hex << (unsigned int)cc[0] << " " << (unsigned int)cc[1] << " " << (unsigned int)cc[2] << " " << (unsigned int)cc[3] << "\n";
						}
					}
				}
			}

		}

		//now we need to match the number coloring, which marks the colors with the numbers 0-3
		//we need to make the corresponding bit maps
		//remember that the bit maps are upside-down
		std::array< uint8_t, 8 > bit0; //<-- controls bit 0 of the color index
		std::array< uint8_t, 8 > bit1; //<-- controls bit 1 of the color index
		int curr_row_id = 0;
		for (int y=7;y>=0;y--){
			uint8_t bit0_row = 0;
			uint8_t bit1_row = 0;

			for (int x=0;x<8;x++){
				int color_number = color_numbering[8 * y + x];
				uint8_t color_bit0 = color_number & 1;
				uint8_t color_bit1 = color_number >> 1 & 1;
				
				bit0_row += color_bit0 << x;
				bit1_row += color_bit1 << x;
			}

			bit0[curr_row_id] = bit0_row;
			bit1[curr_row_id] = bit1_row;
			curr_row_id++;
		}

		PPU466::Tile tile{bit0, bit1};
		ppu.tile_table[i] = tile;
	}

	//hard-code the Color Palettes
	//first color is always transparent
	enum : uint8_t{
		spider_alive,
		spider_dead,
		raindrop,
		grass,
		ground,
		sky,
		cloud
	};

	ppu.palette_table[spider_alive] = {//transparent, red, black, light black
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0xf7, 0x02, 0x02, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0x1b, 0x1b, 0x1b, 0xff)
	};
	ppu.palette_table[spider_dead] = {//transparent, maroon, black, light black
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x4a, 0x12, 0x12, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0x1b, 0x1b, 0x1b, 0xff)
	};
	ppu.palette_table[raindrop] = {//transparent, light blue, medium blue, blue
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x84, 0xb8, 0xf8, 0xff),
		glm::u8vec4(0x59, 0xa0, 0xf7, 0xff),
		glm::u8vec4(0x26, 0x84, 0xf7, 0xff)
	};
	ppu.palette_table[grass] = {//transparent, light green, green, (unused)
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x21, 0x8a, 0x05, 0xff),
		glm::u8vec4(0x22, 0x70, 0x0d, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00)
	};
	ppu.palette_table[ground] = {//transparent, green, (unused), (unused)
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x24, 0x87, 0x09, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00)
	};
	ppu.palette_table[sky] = {//transparent, sky blue, (unused), (unused)
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0xc9, 0xfa, 0xf0, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00)		
	};
	ppu.palette_table[cloud] = {//transparent, white, gray, (unused)
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0xff, 0xff, 0xff, 0xff),
		glm::u8vec4(0xbb, 0xb4, 0xb4, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00)
	};

	//set the background
	//bottom two rows are ground, the rest is sky
	uint16_t sky_background =  (sky << 8) | 5;
	uint16_t ground_background = (ground << 8) | 4;
	for (int i=0;i<ppu.BackgroundWidth * 2;i++){
		ppu.background[i] = ground_background;
	}
	for (int i=ppu.BackgroundWidth * 2;i<ppu.BackgroundWidth * ppu.BackgroundHeight;i++){
		ppu.background[i] = sky_background;
	}

	//set the sprites
	//player (spider alive) sprite
	//set the position of this sprite in PlayMode.cpp
	ppu.sprites[0].x = 0;
	ppu.sprites[0].y = 0;
	ppu.sprites[0].index = 0;
	ppu.sprites[0].attributes = spider_alive;

	//player (spider dead) sprite
	//hidden initially and then swapped in when the player dies
	ppu.sprites[1].x = 16;
	ppu.sprites[1].y = 16;
	ppu.sprites[1].index = 1;
	ppu.sprites[1].attributes = spider_dead;

	//cloud sprites - we have 5 of them
	//they take up sprite slots 2-6
	for (int i=0;i<5;i++){
		ppu.sprites[i+2].x = i * 16 + 32;
		ppu.sprites[i+2].y = i * 16 + 32;
		ppu.sprites[i+2].x = 32 * i + 16;
		ppu.sprites[i+2].y = 208;
		ppu.sprites[i+2].index = 6;
		ppu.sprites[i+2].attributes = cloud;
	}

	//raindrop sprites - we have 10 of these, two for each cloud
	//hidden initially and then teleported to drop underneath a cloud
	//they take up sprite slots 7-16
	for (int i=0;i<10;i++){
		ppu.sprites[i+7].x = i * 16 + 112;
		ppu.sprites[i+7].y = i * 16 + 112;
		ppu.sprites[i+7].index = 2;
		ppu.sprites[i+7].attributes = raindrop;
	}
}