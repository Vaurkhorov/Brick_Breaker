#include <cmath>
#include <iostream>
#include <utility>
#include <map>
#include <limits>
#include <functional>


#include <GL/glut.h>
#include <GL/gl.h>

using std::map;
using std::pair;

void init();
void plot();
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

void plot() {
	glClear(GL_COLOR_BUFFER_BIT);

	

 //   house(300, 200, 100, 50, 200, 600);
	//fish(500, 500, 100, 50, 50);
	
	//mid_point_ellipse(500, 500, 200, 300);
	glFlush();
}


// BRICK START
constexpr int brick_length = 2000;
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

void normal_hit() {
	//larry.step = 2;
}

struct Brick {
	std::function<void()> on_hit = normal_hit;

	// colours
	float r = 0.5;
	float g = 0.5;
	float b = 0.5;
};

struct BreakeableBricks {
	map<pair<int, int>, Brick> bricks;
} bricks_to_break;

void larry_go() {
	larry.y += larry.down ? (-larry.step) : (+larry.step);

	larry.x += larry.direction;

	//switch (larry.direction) {
	//case PlayerState::Left:
	//	larry.x -= larry.step;
	//	break;
	//case PlayerState::Right:
	//	larry.x += larry.step;
	//	break;
	//default:
	//	break;
	//}

	//larry.x += (larry.direction == PlayerState::Left) ? (-larry.step) : (larry.step);

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

			//if (larry.x > player_x + player_length / 2) larry.direction = PlayerState::Right;
			//else larry.direction = PlayerState::Left;
		}
	}

	if (larry.y >= bricks_start_y) {
		if ((larry.y - bricks_start_y) % brick_width <= 50 || (larry.y - bricks_start_y) % brick_width >= brick_width - 50) {
			auto pair_to_check = std::make_pair(((larry.x / brick_length) * brick_length), larry.y / brick_width * brick_width);
			auto it = bricks_to_break.bricks.find(pair_to_check);
			if (it != bricks_to_break.bricks.end()) {
				the_state.hit();
				larry.down = !larry.down;
				bricks_to_break.bricks[pair_to_check].on_hit();
				bricks_to_break.bricks.erase(it);
			}
		}
	}
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

	golden.r = colour(0xFF);
	golden.g = colour(0xD7);
	golden.b = colour(0x00);
	golden.on_hit = []() -> void {
		larry.step = 10;
		the_state.slow_bounce = 5;
	};

	green.r = colour(0x22);
	green.g = colour(0xFF);
	green.b = colour(0x55);
	green.on_hit = []() -> void {
		player_length = default_player_length + green_powerup_length_difference;
		the_state.big_player = green_powerup_duration;
	};

	int flippy = 0;
	for (int i = bricks_start_y; i < ortho_y; i += brick_width) {
		for (int j = 0; j < ortho_x; j += brick_length) {
			float rng = random();
			if (rng > 0.5) continue;
			else if (rng > 0.48) bricks_to_break.bricks.insert({ std::make_pair(j, i), golden });
			else if (rng > 0.46) bricks_to_break.bricks.insert({ std::make_pair(j, i), green });
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

/// <summary>
/// Use within a GL_QUADS block.
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
//void draw_player(int x, int y) {
//	glVertex2d(x, y);
//	glVertex2d(x + brick_length, y);
//	glVertex2d(x + brick_length, y + brick_width);
//	glVertex2d(x, y + brick_width);
//}

void draw_brick(int x, int y, const Brick& brick) {
	glColor4f(brick.r, brick.g, brick.b, 1.0);
	glVertex2d(x, y);
	glVertex2d(x + brick_length, y);
	glVertex2d(x + brick_length, y + brick_width);
	glVertex2d(x, y + brick_width);
}

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
	}

	if (winnerrr) {
		if (((float)rand() / RAND_MAX) * 1000 > 950) {
			larry.down = !larry.down;

			glPointSize((float)rand() / RAND_MAX * 100.0);

			larry.r = (float)rand() / RAND_MAX;
			larry.g = (float)rand() / RAND_MAX;
			larry.b = (float)rand() / RAND_MAX;
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

