#pragma once

#include <types.hpp>

#include <string_view>

void play_local(Player first_player, i32 max_depth);
void play_online(std::string_view ip, std::string_view port, Player player,
                 i32 max_depth);
