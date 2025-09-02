#include <graphics.h>
#include <string>
#include <vector>

inline void putimage_alpha(int x, int y, IMAGE* img);

class Animation
{
public:

	// pathΪͼƬ·��, numΪ��ǰ������ʹ�õ�ͼƬ����, intervalΪ֡���
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

	// x, y��������Ⱦλ��, delta���������һ�ε���Play�����ļ��
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

	// ������ʱ��
	//int timer = 0;

	// �������ż�����
	int counter = 0;

	// ����֡����
	int idx_frame = 0;
	int interval_ms = 0;
	std::vector<IMAGE*> frames;
};

const int PLAYER_ANIM_FRAME_COUNT = 6;

// ��ҿ��
const int PLAYER_WIDTH = 80;
// ��Ҹ߶�
const int PLAYER_HEIGHT = 80;

// ��Ӱ���
const int SHADOW_WIDTH = 32;

// ���ڿ��
const int WINDOW_WIDTH = 1280;
// ���ڸ߶�
const int WINDOW_HEIGHT = 720;

IMAGE img_shadow;

// ���λ������
POINT player_pos = { 500, 500 };

const int PLAYER_SPEED = 5;

Animation anim_left_player(L"img/player_left_%d.png", PLAYER_ANIM_FRAME_COUNT, 45);
Animation anim_right_player(L"img/player_right_%d.png", PLAYER_ANIM_FRAME_COUNT, 45);

#pragma comment(lib, "MSIMG32.LIB")

// putimage����Ⱦ������û��ʹ��IMAGE�����͸������Ϣ, ���ƴ���͸���ȵ�ͼƬʱ, ��Ҫʹ�����º���
inline void putimage_alpha(int x, int y, IMAGE* img)
{
    int w = img->getwidth();
    int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h, GetImageHDC(img), 0, 0, w, h, {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA});
}

void DrawPlayer(int delta, int direction_x)
{
	// Ϊ��һ�����Ӱ
	int pos_shadow_x = player_pos.x + (PLAYER_WIDTH - SHADOW_WIDTH) / 2;
	int pos_shadow_y = player_pos.y + PLAYER_HEIGHT - 8;
	putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

	static bool facing_left = false;
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
		anim_left_player.Play(player_pos.x, player_pos.y, delta);
	}
	else
	{
		anim_right_player.Play(player_pos.x, player_pos.y, delta);
	}
}

int main()
{
	initgraph(1280, 720);

	bool running = true;

	ExMessage msg;
	IMAGE img_bg;

	bool is_move_up = false;
    bool is_move_down = false;
    bool is_move_left = false;
    bool is_move_right = false;

	loadimage(&img_bg, _T("img/background.png"));
	loadimage(&img_shadow, _T("img/shadow_player.png"));

	BeginBatchDraw();

    while (running)
    {
		DWORD start_time = GetTickCount();

		while (peekmessage(&msg))
		{
			if (msg.message == WM_KEYDOWN)
			{
				switch (msg.vkcode)
				{
				case VK_ESCAPE:
					running = false;
					break;
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
			} else if (msg.message == WM_KEYUP)
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

		// ��������ƶ���Χ
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

		cleardevice();

		putimage(0, 0, &img_bg);
		DrawPlayer(1000 / 60, is_move_right - is_move_left);

		FlushBatchDraw();

		DWORD end_time = GetTickCount();
		DWORD delta_time = end_time - start_time;
        Sleep(max(1000 / 60 - static_cast<int>(delta_time), 0));

    }
    EndBatchDraw();
    closegraph();
	return 0;
}