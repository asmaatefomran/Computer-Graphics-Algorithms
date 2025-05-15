#include <windows.h>
#include <cmath>
#include <vector>
#include <ctime>

struct PointF {
    float x, y;
};

struct ColorF {
    float r, g, b;
};

float mix(float a, float b, float t) {
    return a + (b - a) * t;
}

ColorF mixColor(const ColorF& c1, const ColorF& c2, float t) {
    return {
            mix(c1.r, c2.r, t),
            mix(c1.g, c2.g, t),
            mix(c1.b, c2.b, t)
    };
}

ColorF bezierColor(const ColorF& c0, const ColorF& c1, const ColorF& c2, const ColorF& c3, float t) {
    ColorF c01 = mixColor(c0, c1, t);
    ColorF c12 = mixColor(c1, c2, t);
    ColorF c23 = mixColor(c2, c3, t);
    ColorF c012 = mixColor(c01, c12, t);
    ColorF c123 = mixColor(c12, c23, t);
    return mixColor(c012, c123, t);
}

PointF bezierPoint(const PointF& p0, const PointF& p1, const PointF& p2, const PointF& p3, float t) {
    float u = 1 - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    return {
            uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x,
            uuu * p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y
    };
}

void DrawBezierWithColor(HDC hdc, PointF p0, PointF p1, PointF p2, PointF p3,
                         ColorF c0, ColorF c1, ColorF c2, ColorF c3, int steps = 1000) {
    for (int i = 0; i <= steps; ++i) {
        float t = (float)i / steps;
        PointF pt = bezierPoint(p0, p1, p2, p3, t);
        ColorF ct = bezierColor(c0, c1, c2, c3, t);

        COLORREF color = RGB(
                static_cast<int>(ct.r * 255),
                static_cast<int>(ct.g * 255),
                static_cast<int>(ct.b * 255)
        );

        SetPixel(hdc, static_cast<int>(pt.x), static_cast<int>(pt.y), color);
    }
}

// Global state
std::vector<PointF> bezierPoints;
std::vector<ColorF> bezierColors;

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_RBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            bezierPoints.push_back({ (float)x, (float)y });
            bezierColors.push_back({
                                           static_cast<float>(rand() % 100) / 100,
                                           static_cast<float>(rand() % 100) / 100,
                                           static_cast<float>(rand() % 100) / 100
                                   });

            if (bezierPoints.size() == 4) {
                HDC hdc = GetDC(hwnd);
                DrawBezierWithColor(hdc,
                                    bezierPoints[0], bezierPoints[1], bezierPoints[2], bezierPoints[3],
                                    bezierColors[0], bezierColors[1], bezierColors[2], bezierColors[3]);
                ReleaseDC(hwnd, hdc);

                bezierPoints.clear();
                bezierColors.clear();
            }

            break;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    srand(static_cast<unsigned int>(time(NULL))); // Initialize random seed

    const char CLASS_NAME[] = "BezierWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HWND hwnd = CreateWindowEx(
            0, CLASS_NAME, "Bezier Curve Drawer",
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
            NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}
