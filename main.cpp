#include <graphics.h>
#include <string>
#include <vector>

inline void putimage_alpha(int x, int y, IMAGE* img);

// 窗口宽度
const int WINDOW_WIDTH = 1280;
// 窗口高度
const int WINDOW_HEIGHT = 720;

class Animation
{
public:

	// path为图片路径, num为当前动画所使用的图片数量, interval为帧间隔
	Animation(LPCTSTR path, int num, int interval)
	{
		interval_ms = interval;

		TCHAR path_file[256];
		for (size_t i = 0; i < num; i++)
		{
			_stprintf_s(path_file, path, i);
			IMAGE* frame = new IMAGE();
			loadimage(frame, path_file);
			frames.push_back(frame);
		}
	}
	~Animation()
	{
		for (size_t i = 0; i < frames.size(); i++)
		{
			delete frames[i];
		}
	}

	// x, y代表动画渲染位置, delta代表距离上一次调用Play函数的间隔
	void Play(int x, int y, int delta)
	{
		counter += delta;

		if (counter >= interval_ms)
		{
			idx_frame = (idx_frame + 1) % frames.size();
			counter = 0;
		}
		putimage_alpha(x, y, frames[idx_frame]);
	}
private:

	// 动画计时器
	//int timer = 0;

	// 动画播放计数器
	int counter = 0;

	// 动画帧索引
	int idx_frame = 0;
	int interval_ms = 0;
	std::vector<IMAGE*> frames;
};

class Player
{
public:

	Player()
	{
		loadimage(&img_shadow, _T("img/shadow_player.png"));
		anim_left = new Animation(L"img/player_left_%d.png", PLAYER_ANIM_FRAME_COUNT, 45);
		anim_right = new Animation(L"img/player_right_%d.png", PLAYER_ANIM_FRAME_COUNT, 45);
	}

	~Player()
	{
		delete anim_left;
		delete anim_right;
	}

	void ProcessEvent(const ExMessage& msg)
	{
		if (msg.message == WM_KEYDOWN)
		{
			switch (msg.vkcode)
			{
			case VK_LEFT:
				is_move_left = true;
				break;
			case VK_RIGHT:
				is_move_right = true;
				break;
			case VK_UP:
				is_move_up = true;
				break;
			case VK_DOWN:
				is_move_down = true;
				break;
			}
		}
		else if (msg.message == WM_KEYUP)
		{
			switch (msg.vkcode)
			{
			case VK_LEFT:
				is_move_left = false;
				break;
			case VK_RIGHT:
				is_move_right = false;
				break;
			case VK_UP:
				is_move_up = false;
				break;
			case VK_DOWN:
				is_move_down = false;
				break;
			}
		}
	}

	void Move()
	{
		int dir_x = is_move_right - is_move_left;
		int dir_y = is_move_down - is_move_up;
		double input_magnitude = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (input_magnitude != 0)
		{
			double normalize_x = dir_x / input_magnitude;
			double normalize_y = dir_y / input_magnitude;
			player_pos.x += static_cast<int>(normalize_x * PLAYER_SPEED);
			player_pos.y += static_cast<int>(normalize_y * PLAYER_SPEED);
		}

		// 限制玩家移动范围
		if (player_pos.x < 0)
		{
			player_pos.x = 0;
		}
		if (player_pos.x > WINDOW_WIDTH - PLAYER_WIDTH)
		{
			player_pos.x = WINDOW_WIDTH - PLAYER_WIDTH;
		}
		if (player_pos.y < 0)
		{
			player_pos.y = 0;
		}
		if (player_pos.y > WINDOW_HEIGHT - PLAYER_HEIGHT)
		{
			player_pos.y = WINDOW_HEIGHT - PLAYER_HEIGHT;
		}
	}

	void Draw(int delta)
	{
		// 为玩家绘制阴影
		int pos_shadow_x = player_pos.x + (PLAYER_WIDTH - SHADOW_WIDTH) / 2;
		int pos_shadow_y = player_pos.y + PLAYER_HEIGHT - 8;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

		static bool facing_left = false;
		int direction_x = is_move_right - is_move_left;
		if (direction_x < 0)
		{
			facing_left = true;
		}
		else if (direction_x > 0)
		{
			facing_left = false;
		}

		if (facing_left)
		{
			anim_left->Play(player_pos.x, player_pos.y, delta);
		}
		else
		{
			anim_right->Play(player_pos.x, player_pos.y, delta);
		}
	}

	const POINT& GetPlayerPos() const
	{
		return player_pos;
	}

private:
	const int PLAYER_ANIM_FRAME_COUNT = 6;

	const int PLAYER_SPEED = 5;
	// 玩家宽度
	const int PLAYER_WIDTH = 80;
	// 玩家高度
	const int PLAYER_HEIGHT = 80;
	// 阴影宽度
	const int SHADOW_WIDTH = 32;

private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	// 玩家位置坐标
	POINT player_pos = { 500, 500 };
	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;
};

class Enemy
{
public:
    Enemy()
    {
        loadimage(&img_shadow, _T("img/shadow_enemy.png"));
        anim_left = new Animation(L"img/enemy_left_%d.png", ENEMY_ANIM_FRAME_COUNT, 45);
        anim_right = new Animation(L"img/enemy_right_%d.png", ENEMY_ANIM_FRAME_COUNT, 45);

		enum class SpawnEdge
		{
            UP = 0,
            DOWN = 1,
            LEFT = 2,
            RIGHT = 3
		};

		// 将敌人放置在地图边缘的随机位置
		SpawnEdge edge = static_cast<SpawnEdge>(rand() % 4);
		switch (edge)
		{
            case SpawnEdge::UP:
                enemy_pos.x = rand() % WINDOW_WIDTH;
                enemy_pos.y = -ENEMY_HEIGHT;
                break;
            case SpawnEdge::DOWN:
                enemy_pos.x = rand() % WINDOW_WIDTH;
                enemy_pos.y = WINDOW_HEIGHT;
                break;
            case SpawnEdge::LEFT:
                enemy_pos.x = -ENEMY_WIDTH;
                enemy_pos.y = rand() % WINDOW_HEIGHT;
                break;
			case SpawnEdge::RIGHT:
                enemy_pos.x = WINDOW_WIDTH;
                enemy_pos.y = rand() % WINDOW_HEIGHT;
                break;
			default:
                break;
		}

    }
    ~Enemy()
    {
        delete anim_left;
        delete anim_right;
    }

	bool CheckPlayerCollision(const Player& player)
	{
		return false;
	}

	void Move(const Player& player)
	{ 
		const POINT& player_pos = player.GetPlayerPos();
		int dir_x = player_pos.x - enemy_pos.x;
        int dir_y = player_pos.y - enemy_pos.y;
		double input_magnitude = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (input_magnitude != 0)
		{
			double normalize_x = dir_x / input_magnitude;
			double normalize_y = dir_y / input_magnitude;
			enemy_pos.x += static_cast<int>(normalize_x * ENEMY_SPEED);
			enemy_pos.y += static_cast<int>(normalize_y * ENEMY_SPEED);
		}
		if (dir_x < 0)
		{
			facing_left = true;
		}
		else if (dir_x > 0)
		{
			facing_left = false;
		}
	}

	void Draw(int delta)
	{
		// 为敌人绘制阴影
		int pos_shadow_x = enemy_pos.x + (ENEMY_WIDTH - SHADOW_WIDTH) / 2;
		int pos_shadow_y = enemy_pos.y + ENEMY_HEIGHT - 35;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);
		if (facing_left)
		{
			anim_left->Play(enemy_pos.x, enemy_pos.y, delta);
		}
		else
		{
			anim_right->Play(enemy_pos.x, enemy_pos.y, delta);
		}
	}

private:
	const int ENEMY_SPEED = 3;
	const int ENEMY_WIDTH = 80;
	const int ENEMY_HEIGHT = 80;
	const int SHADOW_WIDTH = 48;
	const int ENEMY_ANIM_FRAME_COUNT = 6;

private:
    IMAGE img_shadow;
    Animation* anim_left;
    Animation* anim_right;
    POINT enemy_pos = { 0, 0 };
	bool facing_left = false;
};

#pragma comment(lib, "MSIMG32.LIB")

// putimage在渲染过程中没有使用IMAGE对象的透明度信息, 绘制带有透明度的图片时, 需要使用以下函数
inline void putimage_alpha(int x, int y, IMAGE* img)
{
    int w = img->getwidth();
    int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h, GetImageHDC(img), 0, 0, w, h, {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA});
}

void TryGenerateNewEnemy(std::vector<Enemy*>& enemies)
{
	const int ENEMY_SPAWN_INTERVAL = 100;
	static int counter = 0;
	if ((++counter) % ENEMY_SPAWN_INTERVAL == 0)
	{
		enemies.push_back(new Enemy());
	}
}

int main()
{
	initgraph(1280, 720);

	bool running = true;

	Player player;
	ExMessage msg;
	IMAGE img_bg;
	std::vector<Enemy*> enemies;

	loadimage(&img_bg, _T("img/background.png"));

	BeginBatchDraw();

    while (running)
    {
		DWORD start_time = GetTickCount();

		while (peekmessage(&msg))
		{
			player.ProcessEvent(msg);
		}
		player.Move();

		TryGenerateNewEnemy(enemies);
		for (auto enemy : enemies)
        {
			enemy->Move(player);
        }

		cleardevice();

		putimage(0, 0, &img_bg);
		player.Draw(1000 / 60);
        for (auto enemy : enemies)
        {
			enemy->Draw(1000 / 60);
        }

		FlushBatchDraw();

		DWORD end_time = GetTickCount();
		DWORD delta_time = end_time - start_time;
        Sleep(max(1000 / 60 - static_cast<int>(delta_time), 0));

    }
    EndBatchDraw();
    closegraph();
	return 0;
}