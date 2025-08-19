#include <graphics.h>
#include <string>
#include <vector>

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

	// x, y代表动画渲染位置, delta代表距离上一次调用Play函数的时间间隔
	void Play(int x, int y, int delta)
	{
		timer += delta;

		if (timer >= interval_ms)
        {
			idx_frame = (idx_frame + 1) % frames.size();
			timer = 0;
        }
		putimage_alpha(x, y, frames[idx_frame]);
	}
private:

	// 动画计时器
	int timer = 0;

	// 动画帧索引
	int idx_frame = 0;
	int interval_ms = 0;
    std::vector<IMAGE*> frames;
};

// 当前动画的帧索引
int idx_current_anim = 0;

const int PLAYER_ANIM_FRAME_COUNT = 6;

IMAGE img_player_left[PLAYER_ANIM_FRAME_COUNT];
IMAGE img_player_right[PLAYER_ANIM_FRAME_COUNT];

// 玩家位置坐标
POINT player_pos = { 500, 500 };

const int PLAYER_SPEED = 5;

#pragma comment(lib, "MSIMG32.LIB")

// putimage在渲染过程中没有使用IMAGE对象的透明度信息, 绘制带有透明度的图片时, 需要使用以下函数
inline void putimage_alpha(int x, int y, IMAGE* img)
{
    int w = img->getwidth();
    int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h, GetImageHDC(img), 0, 0, w, h, {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA});
}

void LoadAnimation()
{
    for (size_t i = 0; i < PLAYER_ANIM_FRAME_COUNT; i++)
    {
        std::wstring path = L"img/player_left_" + std::to_wstring(i) + L".png";
        loadimage(&img_player_left[i], path.c_str());

		path = L"img/player_right_" + std::to_wstring(i) + L".png";
        loadimage(&img_player_right[i], path.c_str());
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

	LoadAnimation();
	loadimage(&img_bg, _T("img/background.png"));

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

        if (is_move_left)
        {
            player_pos.x -= PLAYER_SPEED;
        }
        if (is_move_right)
        {
            player_pos.x += PLAYER_SPEED;
        }
        if (is_move_up)
        {
            player_pos.y -= PLAYER_SPEED;
        }
        if (is_move_down)
        {
            player_pos.y += PLAYER_SPEED;
        }

		// 使用static确保counter只在第一个游戏帧时被初始化为0
		static int counter = 0;

		// 每5个游戏帧切换一个动画帧
		if (++counter % 5 == 0)
		{
			idx_current_anim++;
		}

		// 使动画循环播放
		idx_current_anim %= PLAYER_ANIM_FRAME_COUNT;

		cleardevice();

		putimage(0, 0, &img_bg);
		putimage_alpha(player_pos.x, player_pos.y, &img_player_left[idx_current_anim]);

		FlushBatchDraw();

		DWORD end_time = GetTickCount();
		DWORD delta_time = end_time - start_time;
        Sleep(max(1000 / 60 - static_cast<int>(delta_time), 0));

    }
    EndBatchDraw();
    closegraph();
	return 0;
}