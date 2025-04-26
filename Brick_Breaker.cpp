#include <cmath>
#include <iostream>
#include <utility>
#include <map>
#include <limits>
#include <functional>
#include <vector>


#include <GL/glut.h>
#include <GL/gl.h>

using std::map;
using std::pair;
using std::vector;

void init();
void idle();

float colour(float hex) {
	return hex / 255;
}

float random() {
	return (float)rand() / RAND_MAX;
}

void brick_init();
void brick_breaker();
void brickKeypress(unsigned char key, int x, int y);
void brickKeyrelease(unsigned char key, int x, int y);
void brickSpecialKeypress(int key, int x, int y);
void brickSpecialKeyRelease(int key, int x, int y);

struct BreakeableBricks;
struct Entity;


int main(int argc, char** argv)
{
	srand(time(NULL));
	using namespace std;

	cout << "Press 'a' and 'd' to move left or right." << endl;
	cout << "Game over? Press 'r' to restart." << endl;
	cout << "Press 'q' to quit." << endl;
	cout << endl << "Press enter to start. Good luck!" << endl;
	cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	int mode = GLUT_SINGLE | GLUT_RGB;
	glutInit(&argc, argv);
	glutInitDisplayMode(mode);
	glutInitWindowSize(700, 700);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("3878");
	brick_init();

	glutKeyboardFunc(brickKeypress);
	glutKeyboardUpFunc(brickKeyrelease);
	glutSpecialFunc(brickSpecialKeypress);
	glutSpecialUpFunc(brickSpecialKeyRelease);
	glutDisplayFunc(brick_breaker);
	glutIdleFunc(idle);

	glutMainLoop();
}

void init() {
	glClearColor(0.0, 0.25, 0.25, 1.0);
	glColor3f(0.2, 0.8, 1.0);
	glPointSize(1.0);
	glLineWidth(2.0);
	glViewport(0, 0, 1000, 1000);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 1000, 0, 1000);
	glClear(GL_COLOR_BUFFER_BIT);
}

void idle() {
	glutPostRedisplay();
}


// BRICK START
constexpr int brick_length = 1000;
constexpr int brick_width = 200;

constexpr int ortho_x = 10000;
constexpr int ortho_y = 10000;

constexpr int default_player_length = 2000;
int player_length = default_player_length;
constexpr int player_x_min = 0;
std::function<int()> player_x_max = []() -> int {
	return ortho_x - player_length;
	};
constexpr int player_step = 30;

constexpr int green_powerup_length_difference = default_player_length;
constexpr int green_powerup_duration = 10;

constexpr int bricks_start_y = ortho_y * 0.6;

int player_x = 4500;
int player_y = 1000;

int draw_delay = 2;

bool game_over = false;


enum class PlayerState {
	Left,
	Right,
	Still,
};
PlayerState player_state = PlayerState::Still;

struct breaker {
	int size = 10;

	int x = 5000;
	int y = 5000;

	bool down = false;
	int direction = 0;

	int step = 20;

	// colours
	float r = 1.0;
	float g = 1.0;
	float b = 1.0;
} larry, sleepy_barry;	// sleepy_barry holds the default settings

struct game_state {
	bool eww_this_isnt_functional = true;

	int player_step = 20;

	int slow_bounce = 0;
	int big_player = 0;

	vector<Entity> entities = vector<Entity>();

	void hit() {
		if (slow_bounce > 0) {
			slow_bounce--;
		}
		
		if (slow_bounce == 0) {
			larry.step = sleepy_barry.step;
		}


		if (big_player > 0) {
			big_player--;
			player_length -= green_powerup_length_difference / green_powerup_duration;
		}

		if (big_player == 0) {
			player_length = default_player_length;
		}
	}
} the_state;

void normal_hit(pair<int, int>, BreakeableBricks&) {
	// do nothing
}

struct Brick {
	std::function<void(pair<int, int>, BreakeableBricks& bricks_to_break)> on_hit = normal_hit;

	// colours
	float r = 0.5;
	float g = 0.5;
	float b = 0.5;
};

struct BreakeableBricks {
	map<pair<int, int>, Brick> bricks;
} bricks_to_break;

map<int, vector<int>> entity_memory;
int current_memory_id = 0;


struct Entity {
	int x = 0;
	int y = 0;

	float r = 1.0;
	float g = 1.0;
	float b = 1.0;



	// I like to think that this is a little clever.
	// Each entity essentially has its own memory in the form of integer arrays.
	// This memory is in no way safe, but it isn't meant to be.
	// But it allows any function to store basically arbitrary data as needed.
	int memory_id;
	Entity() {
		memory_id = current_memory_id++;
	}


	// The member functions below take an entity object as an argument,
	// even though they aren't static.
	// I did this for two reasons. I'm not fully sure if the reasons are correct, though.
	// One, they aren't member functions, and can't access other member variables normally.
	// Two, there *is* a possibility that the function is copied and stored somewhere like a normal variable.
	// So if I were to capture the object within it, the function could outlive the object somehow.
	// It is an unlikely situation. Why would you copy the function?
	// But if it does happen, say, to defer execution, aur kahi koi dangling reference reh gaya,
	// toh I believe larry ka sutli bomb ban jaayega. >:3

	std::function<void(Entity&)> move = [](Entity&) -> void {
		// do nothing
		};

	std::function<void(Entity&)> draw = [](Entity& self) -> void {
		glBegin(GL_POINTS);
		glColor4f(self.r, self.g, self.b, 1.0);
		glVertex2i(self.x, self.y);
		glEnd();
		};

	/// <summary>
	/// If it returns false, the entity is deleted.
	/// </summary>
	std::function<bool(Entity&)> hit_border = [](Entity& self) -> bool {
		return false;
		};

	std::function<bool(Entity&)> on_hit_player = [](Entity& self) -> bool {
		return false;
		};

	std::function<bool(Entity&, pair<int, int>, BreakeableBricks&)> on_hit_block = [](Entity&, pair<int, int>, BreakeableBricks&) -> bool {
		return false;
		};
};

void larry_go() {
	larry.y += larry.down ? (-larry.step) : (+larry.step);

	larry.x += larry.direction;

	if (larry.x < 0) {
		larry.x = 0;
		larry.direction *= -1;
	}

	if (larry.x > ortho_x - larry.size) {
		larry.x = ortho_x - larry.size;
		larry.direction *= -1;
	}

	if (larry.y > ortho_y - larry.size) {
		larry.y = ortho_y - larry.size;
		larry.down = true;
	}

	if (larry.y < 0) {
		larry.y = 0;
		larry.down = false;
		bricks_to_break.bricks.clear();
		game_over = true;
		return;
	}

	if (!game_over && larry.y <= player_y + larry.size * 10 && larry.y >= player_y - larry.size * 5) {
		if (larry.x > player_x && larry.x < player_x + player_length) {
			if (larry.down) larry.down = false;

			larry.direction = (larry.x - (player_x + player_length / 2)) * 70 / player_length;
		}
	}

	if (larry.y >= bricks_start_y) {
		if ((larry.y - bricks_start_y) % brick_width <= 50 || (larry.y - bricks_start_y) % brick_width >= brick_width - 50) {
			auto pair_to_check = std::make_pair(((larry.x / brick_length) * brick_length), larry.y / brick_width * brick_width);
			auto it = bricks_to_break.bricks.find(pair_to_check);
			if (it != bricks_to_break.bricks.end()) {
				the_state.hit();
				larry.down = !larry.down;
				
				auto hit_brick = bricks_to_break.bricks[pair_to_check];
				bricks_to_break.bricks.erase(it);
				hit_brick.on_hit(pair_to_check, bricks_to_break);
			}
		}
	}
}

void handle_entities() {
	// Don't draw them here, that'll be handled by a separate function.
	vector<Entity> retained_entities;
	for (Entity& it : the_state.entities) {
		it.move(it);

		if (it.x < 0 || it.x > ortho_x || it.y < 0 || it.y > ortho_y) {
			if (it.hit_border(it)) {
				retained_entities.push_back(it);
				continue;
			}
		}

		if (it.x > player_x && it.x < player_x + player_length && it.y > player_y && it.y < player_y + brick_width) {
			if (it.on_hit_player(it)) {
				retained_entities.push_back(it);
				continue;
			}
		}
		
		if (it.y >= bricks_start_y) {
			if ((it.y - bricks_start_y) % brick_width <= 50 || (it.y - bricks_start_y) % brick_width >= brick_width - 50) {
				auto pair_to_check = std::make_pair(((it.x / brick_length) * brick_length), it.y / brick_width * brick_width);
				if (bricks_to_break.bricks.find(pair_to_check) != bricks_to_break.bricks.end()) {
					if (it.on_hit_block(it, pair_to_check, bricks_to_break)) {
						retained_entities.push_back(it);
						continue;
					}
				}
			}
		}

		retained_entities.push_back(it);
	}

	the_state.entities.clear();
	the_state.entities = std::move(retained_entities);
}

void player_move() {
	switch (player_state) {
	case PlayerState::Left:
		player_x = (player_x_min < player_x - the_state.player_step) ? (player_x - the_state.player_step) : player_x_min;
		break;
	case PlayerState::Right:
		player_x = (player_x_max() > player_x + the_state.player_step) ? (player_x + the_state.player_step) : player_x_max();
		break;
	case PlayerState::Still:
		break;
	}
}

void setup_game() {
	glPointSize(larry.size);

	player_x = 4500;
	player_y = 1000;
	player_length = default_player_length;

	the_state = game_state();

	larry.x = 5000;
	larry.y = 5000;
	larry.down = false;
	larry.r = 1.0;
	larry.g = 1.0;
	larry.b = 1.0;
	larry.direction = 0;
	larry.step = 20;

	draw_delay = 2;

	game_over = false;

	bricks_to_break.bricks.clear();

	Brick normal;
	Brick golden;
	Brick green;
	Brick red;
	Brick blue;

	Entity player_bullet;

	golden.r = colour(0xFF);
	golden.g = colour(0xD7);
	golden.b = colour(0x00);
	golden.on_hit = [](pair<int, int>, BreakeableBricks&) -> void {
		larry.step = 10;
		the_state.slow_bounce = 5;
	};

	green.r = colour(0x22);
	green.g = colour(0xFF);
	green.b = colour(0x55);
	green.on_hit = [](pair<int, int>, BreakeableBricks&) -> void {
		player_length = default_player_length + green_powerup_length_difference;
		the_state.big_player = green_powerup_duration;
	};

	red.r = colour(0xFF);
	red.g = colour(0x00);
	red.b = colour(0x00);
	red.on_hit = [normal](pair<int, int> current_brick, BreakeableBricks& bricks_to_break) -> void {
		bricks_to_break.bricks.erase(current_brick);
		bricks_to_break.bricks.insert({ current_brick, normal });
	};

	player_bullet.r = colour(0xFF);
	player_bullet.g = colour(0x00);
	player_bullet.b = colour(0x00);
	player_bullet.on_hit_block = [](Entity&, pair<int, int> brick_pos, BreakeableBricks& bricks_to_break) -> bool {
		bricks_to_break.bricks.erase(bricks_to_break.bricks.find(brick_pos));
		return true;
	};
	player_bullet.on_hit_player = [](Entity&) -> bool {
		return true;
	};
	player_bullet.hit_border = [](Entity&) -> bool {
		return false;
	};
	player_bullet.move = [](Entity& bullet) -> void {
		bullet.y += 10;
		bullet.x += (entity_memory[bullet.memory_id])[0];
	};
	player_bullet.draw = [](Entity& bullet) -> void {
		glColor3f(bullet.r, bullet.g, bullet.b);
		glVertex2i(bullet.x, bullet.y);
	};

	blue.r = colour(0x00);
	blue.g = colour(0x00);
	blue.b = colour(0xFF);
	blue.on_hit = [player_bullet](pair<int, int>, BreakeableBricks& bricks_to_break) -> void {
		Entity bullet = player_bullet;
		bullet.memory_id = current_memory_id++;
		bullet.x = player_x + player_length / 2;
		bullet.y = player_y;

		bullet.r = colour(random() * 0xFF / 2 + 0xFF / 2);
		bullet.g = colour(random() * 0xFF / 2 + 0xFF / 2);
		bullet.b = colour(random() * 0xFF / 2 + 0xFF / 2);

		entity_memory[bullet.memory_id].push_back(((int)(random() * 20) - 10));

		the_state.entities.push_back(bullet);
	};

	int flippy = 0;
	for (int i = bricks_start_y; i < ortho_y; i += brick_width) {
		for (int j = 0; j < ortho_x; j += brick_length) {
			float rng = random();
			if (rng > 0.5) continue;
			else if (rng > 0.48) bricks_to_break.bricks.insert({ std::make_pair(j, i), golden });
			else if (rng > 0.46) bricks_to_break.bricks.insert({ std::make_pair(j, i), green });
			else if (rng > 0.44) bricks_to_break.bricks.insert({ std::make_pair(j, i), red });
			else if (rng > 0.32) bricks_to_break.bricks.insert({ std::make_pair(j, i), blue });
			else bricks_to_break.bricks.insert({ std::make_pair(j, i), normal });
		}
		flippy = (int)!flippy;
	}
}

void brick_init() {
	glClearColor(0.0, 0.25, 0.25, 1.0);
	glColor3f(0.2, 0.8, 1.0);
	glPointSize(larry.size);
	glLineWidth(2.0);
	glViewport(0, 0, ortho_x, ortho_y);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, ortho_x, 0, ortho_y);
	glClear(GL_COLOR_BUFFER_BIT);

	setup_game();
}

void draw_brick(int x, int y, const Brick& brick) {
	glColor4f(brick.r, brick.g, brick.b, 1.0);
	glVertex2d(x, y);
	glVertex2d(x + brick_length, y);
	glVertex2d(x + brick_length, y + brick_width);
	glVertex2d(x, y + brick_width);
}

/// <summary>
/// Use within a GL_QUADS block.
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
void draw_player() {
	glColor4f(colour(0xFF), colour(0xFF), colour(0xFF), 1.0);
	glVertex2d(player_x, player_y);
	glVertex2d(player_x + player_length, player_y);
	glVertex2d(player_x + player_length, player_y + brick_width);
	glVertex2d(player_x, player_y + brick_width);
}

void brick_breaker() {
	glClear(GL_COLOR_BUFFER_BIT);

	player_move();
	larry_go();
	handle_entities();

	bool winnerrr = bricks_to_break.bricks.empty() && !game_over;

	if (!game_over && !winnerrr) {
		glColor4f(colour(0xAA), colour(0xAA), colour(0xFF), 1.0);
		glBegin(GL_QUADS);
		draw_player();
		glColor4f(colour(0xFF), colour(0x55), colour(0x55), 1.0);
		for (auto& it : bricks_to_break.bricks) {
			draw_brick(it.first.first, it.first.second, it.second);
		}
		glEnd();

		glBegin(GL_POINTS);
		for (Entity& it : the_state.entities) {
			it.draw(it);
		}
		glEnd();
	}

	if (winnerrr) {
		if (larry.y < 30) {
			larry.y = 30;
		}
		glPointSize(sleepy_barry.size);
		glBegin(GL_POINTS);
		for (Entity& it : the_state.entities) {
			it.draw(it);
		}
		glEnd();

		if (random() * 1000 > 950) {
			larry.down = !larry.down;

			glPointSize((float)rand() / RAND_MAX * 100.0);

			larry.r = (float)rand() / RAND_MAX;
			larry.g = (float)rand() / RAND_MAX;
			larry.b = (float)rand() / RAND_MAX;

			//if (random() * 10 > 7) {
				Entity confetti;
				confetti.memory_id = current_memory_id++;
				confetti.x = larry.x;
				confetti.y = larry.y;

				confetti.r = colour(random() * 0xFF / 2 + 0xFF / 2);
				confetti.g = colour(random() * 0xFF / 2 + 0xFF / 2);
				confetti.b = colour(random() * 0xFF / 2 + 0xFF / 2);

				entity_memory[confetti.memory_id].push_back(((int)(random() * 20) - 10));
				entity_memory[confetti.memory_id].push_back(((int)(random() * 20) - 10));

				confetti.move = [](Entity& entity) -> void {
					entity.x += entity_memory[entity.memory_id][0];
					entity.y += entity_memory[entity.memory_id][1];
				};

				the_state.entities.push_back(confetti);
				if (the_state.entities.size() > 1000) {
					std::reverse(the_state.entities.begin(), the_state.entities.end());
					the_state.entities.resize(100, confetti);
				}
			//}
		}
	}

	glColor4f(larry.r, larry.g, larry.b, 1.0);
	glBegin(GL_POINTS);
	glVertex2i(larry.x, larry.y);
	glEnd();

	Sleep(draw_delay);
	glFlush();
}

void brickKeypress(unsigned char key, int x, int y) {
	switch (key) {
	case 'a':
	case 'A':
	player_state = PlayerState::Left;
		break;
	case 'd':
	case 'D':
		player_state = PlayerState::Right;
		break;
	case 'r':
	case 'R':
		setup_game();
		game_over = false;
		break;
	case '0':
		bricks_to_break.bricks.clear();
		break;
	case 'q':
	case 'Q':
		exit(0);
		break;	// break your leg, that is.
	default:
		break;
	}
}

void brickSpecialKeypress(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_LEFT:
		player_state = PlayerState::Left;
		break;
	case GLUT_KEY_RIGHT:
		player_state = PlayerState::Right;
		break;
	default:
		break;
	}
}

void brickKeyrelease(unsigned char key, int x, int y) {
	switch (key) {
	case 'a':
	case 'A':
		if (player_state == PlayerState::Left) player_state = PlayerState::Still;
		break;
	case 'd':
	case 'D':
		if (player_state == PlayerState::Right) player_state = PlayerState::Still;
		break;
	default:
		break;
	}
}

void brickSpecialKeyRelease(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_LEFT:
		if (player_state == PlayerState::Left) player_state = PlayerState::Still;
		break;
	case GLUT_KEY_RIGHT:
		if (player_state == PlayerState::Right) player_state = PlayerState::Still;
		break;
	default:
		break;
	}
}