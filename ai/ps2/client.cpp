#include <client.hpp>

#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <bot.hpp>
#include <configuration.hpp>

static void print_board(Configuration const& c) {
  for(i32 y = 0; y < Configuration::height; y += 1) {
    // Print horizontal separator.
    if(y != 0) {
      for(i32 x = 1; x < Configuration::width * 2; x += 1) {
        printf("-");
      }
      printf("\n");
    }
    // Print the row with separators inbetween.
    for(i32 x = 0; x < Configuration::width; x += 1) {
      if(x != 0) {
        printf("|");
      }
      State const s = c(x, y);
      switch(s) {
        case State::empty:
          printf(" ");
          break;
        case State::x:
          printf("X");
          break;
        case State::o:
          printf("O");
          break;
      }
    }
    printf("\n");
  }
  printf("\n");
}

void play_local(Player const human_player, i32 const max_depth) {
  Configuration c;
  print_board(c);
  Player const ai_player = OPPONENT(human_player);
  while(true) {
    {
      MINMAX_Parameters p{
        .c = c,
        .player = human_player,
        .turn = human_player,
        .depth = 0,
        .max_depth = max_depth,
        .alpha = minimum_i32,
        .beta = maximum_i32,
      };
      MINMAX_Result const result = minmax_search(p);
      c(result.x, result.y) = PLAYER_TO_STATE(human_player);
      print_board(c);

      std::optional<Player> winner = c.check_winner();
      if(winner) {
        Player const player = winner.value();
        if(player == Player::x) {
          printf("X won!\n");
        } else {
          printf("O won!\n");
        }
        return;
      }

      bool const draw = c.check_draw();
      if(draw) {
        printf("Draw!\n");
        return;
      }
    }
    {
      MINMAX_Parameters p{
        .c = c,
        .player = ai_player,
        .turn = ai_player,
        .depth = 0,
        .max_depth = max_depth,
        .alpha = minimum_i32,
        .beta = maximum_i32,
      };
      MINMAX_Result const result = minmax_search(p);
      c(result.x, result.y) = PLAYER_TO_STATE(ai_player);
      print_board(c);

      std::optional<Player> winner = c.check_winner();
      if(winner) {
        Player const player = winner.value();
        if(player == Player::x) {
          printf("X won!\n");
        } else {
          printf("O won!\n");
        }
        return;
      }

      bool const draw = c.check_draw();
      if(draw) {
        printf("Draw!\n");
        return;
      }
    }
  }
}

void play_online(std::string_view const ip, std::string_view const port,
                 Player const player, i32 const max_depth) {
  // Author: Maciej Gębala
  // License: CC BY-NC 4.0
  // Modifications (Piotr Kocia):
  //   Rewritten from a standalone program to a function, the code now calls
  //   MINMAX_search instead of reading input from the user. Adapted to C++.

  Player const opponent = OPPONENT(player);

  // Create socket
  int const socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  if(socket_desc < 0) {
    printf("Unable to create socket\n");
    return;
  }
  printf("Socket created successfully\n");

  struct sockaddr_in server_addr;
  // Set port and IP the same as server-side
  server_addr.sin_family = AF_INET;
  // String views are not neccessarily null-terminated which is a
  // bug/crash/security/etc risk, but this is a temporary code, so don't care.
  server_addr.sin_port = htons(atoi(port.data()));
  server_addr.sin_addr.s_addr = inet_addr(ip.data());

  // Send connection request to server
  if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) <
     0) {
    printf("Unable to connect\n");
    return;
  }
  printf("Connected with server successfully\n");

  char server_message[16], client_message[16];
  // Await server's welcome message.
  memset(server_message, '\0', sizeof(server_message));
  if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0) {
    printf("Error while receiving server's message\n");
    return;
  }
  printf("Server message: %s\n", server_message);

  // Send the player index.
  memset(client_message, '\0', sizeof(client_message));
  client_message[0] = static_cast<i32>(player) + 1 + '0';
  if(send(socket_desc, client_message, strlen(client_message), 0) < 0) {
    printf("Unable to send message\n");
    return;
  }

  Configuration c;
  while(true) {
    memset(server_message, '\0', sizeof(server_message));
    if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0) {
      printf("Error while receiving server's message\n");
      return;
    }
    printf("Server message: %s\n", server_message);
    int const msg = atoi(server_message);
    int const move = msg % 100;
    int const kind = msg / 100;
    if(move != 0) {
      i32 const x = (move / 10) - 1;
      i32 const y = (move % 10) - 1;
      assert(x >= 0 && x < 5 && y >= 0 && y < 5);
      c(x, y) = PLAYER_TO_STATE(opponent);
      print_board(c);
    }

    if((kind == 0) || (kind == 6)) {
      MINMAX_Parameters p{
        .c = c,
        .player = player,
        .turn = player,
        .depth = 0,
        .max_depth = max_depth,
        .alpha = minimum_i32,
        .beta = maximum_i32,
      };
      MINMAX_Result const result = minmax_search(p);
      c(result.x, result.y) = PLAYER_TO_STATE(player);
      print_board(c);

      memset(client_message, '\0', sizeof(client_message));
      sprintf(client_message, "%d%d", result.x + 1, result.y + 1);
      if(send(socket_desc, client_message, strlen(client_message), 0) < 0) {
        printf("Unable to send message\n");
        return;
      }
      printf("Client message: %s\n", client_message);
    } else {
      switch(kind) {
        case 1:
          printf("You won.\n");
          break;
        case 2:
          printf("You lost.\n");
          break;
        case 3:
          printf("Draw.\n");
          break;
        case 4:
          printf("You won. Opponent error.\n");
          break;
        case 5:
          printf("You lost. Your error.\n");
          break;
      }
      break;
    }
  }

  // Close socket
  close(socket_desc);
}
