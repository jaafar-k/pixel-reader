#ifndef FONT_CATALOG_H_
#define FONT_CATALOG_H_

#include <string>

#include <SDL/SDL_ttf.h>

std::string get_valid_font_name(const std::string &preferred_font_name);
std::string get_prev_font_name(const std::string &font_name);
std::string get_next_font_name(const std::string &font_name);

bool font_supports_arabic(TTF_Font *font);
// Prefer Arabic-capable font when reading RTL content.
std::string get_content_font_name(const std::string &preferred_font_name, bool prefer_arabic);

#endif
