#include <windows.h>
#include <cmath>
#include <vector>

struct Circle {
    int xc, yc;
    int R;
};

using namespace std;
// Global variables
vector<Circle> circles;
vector<POINT> clickPoints;
POINT touchPoint;
bool touchPointSet = false;


// Function to draw 8 symmetric points of a circle
void Draw8Points(HDC hdc, int xc, int yc, int x, int y, COLORREF c) {
    SetPixel(hdc, xc + x, yc + y, c);
    SetPixel(hdc, xc - x, yc + y, c);
    SetPixel(hdc, xc + x, yc - y, c);
    SetPixel(hdc, xc - x, yc - y, c);
    SetPixel(hdc, xc + y, yc + x, c);
    SetPixel(hdc, xc - y, yc + x, c);
    SetPixel(hdc, xc + y, yc - x, c);
    SetPixel(hdc, xc - y, yc - x, c);
}


// Function to draw a line using Bresenham's algorithm
void DrawLineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

    float xInc = (float)dx / (float)steps;
    float yInc = (float)dy / (float)steps;

    float x = static_cast<float>(x1);
    float y = static_cast<float>(y1);

    SetPixel(hdc, x1, y1, c);
    for (int i = 0; i < steps; i++) {
        x += xInc;
        y += yInc;
        SetPixel(hdc, round(x), round(y), c);
    }
}
// function works as draw8points but it call the drawDDAline and 2 times only in order to draw in 2 octals(1 quarter)
void Draw2Lines(HDC hdc, int xc, int yc, int x, int y, COLORREF c) {
    DrawLineDDA(hdc, xc+x, yc+y, xc, yc, c);
    DrawLineDDA(hdc, xc-x, yc+y, xc, yc, c);

}

// Function to draw a circle using Bresenham's algorithm
void DrawCircleBres(HDC hdc, int xc, int yc, int R, COLORREF c) {
    int x = 0, y = R;
    int d = 1 - R;
    int d1 = 3, d2 = 5 - 2 * R;
    Draw8Points(hdc, xc, yc, x, y, c);
    while (x < y) {
        if (d < 0) {
            d += d1;
            d1 += 2;
            d2 += 2;
            x++;
        } else {
            d += d2;
            d1 += 2;
            d2 += 4;
            x++;
            y--;
        }
        Draw8Points(hdc, xc, yc, x, y, c);
    }
}



// Function to fill a quadrant of a circle with lines using Bresenham-like approach
void FillQuadrant(HDC hdc, int xc, int yc, int R, COLORREF c) {
    int x = 0, y = R;
    int d = 1 - R;
    int d1 = 3, d2 = 5 - 2 * R;
    Draw2Lines(hdc, xc, yc, x, y, c);
    while (x < y) {
        if (d < 0) {
            d += d1;
            d1 += 2;
            d2 += 2;
            x++;
        } else {
            d += d2;
            d1 += 2;
            d2 += 4;
            x++;
            y--;
        }
        Draw2Lines(hdc, xc, yc, x, y, c);
    }
}

// Function to draw all elements
void DrawAll(HDC hdc) {
    RECT rect;
    GetClientRect(WindowFromDC(hdc), &rect);
    FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));

    // Draw circles
    for (size_t i = 0; i < circles.size(); i++) {
        DrawCircleBres(hdc, circles[i].xc, circles[i].yc, circles[i].R, RGB(0, 0, 0));
    }
    if (touchPointSet) {
        for (int r = 0; r < 5; r++) {
            DrawCircleBres(hdc, touchPoint.x, touchPoint.y, r, RGB(255, 0, 0));
        }
    }

    if (circles.size() == 2 ) {
        FillQuadrant(hdc, circles[0].xc, circles[0].yc, circles[0].R, RGB(128, 0, 128));
    }

    // Draw status text
    TCHAR status[256];
    wsprintf(status, TEXT("Left clicks: %d/2, Right click for touch point: %s"),
             clickPoints.size(), touchPointSet ? "Set" : "Not Set");
    TextOut(hdc, 10, 10, status, lstrlen(status));
}

// Calculate the distance between two points
double Dist(int x1, int y1, int x2, int y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

// Calculate circles based on centers and touch point
void CalculateCircles() {
    circles.clear();

    // Create first circle
    Circle circle1;
    circle1.xc = clickPoints[0].x;
    circle1.yc = clickPoints[0].y;
    circle1.R = (int)Dist(circle1.xc, circle1.yc, touchPoint.x, touchPoint.y);
    circles.push_back(circle1);

    // Create second circle
    Circle circle2;
    circle2.xc = clickPoints[1].x;
    circle2.yc = clickPoints[1].y;
    circle2.R = (int)Dist(circle2.xc, circle2.yc, touchPoint.x, touchPoint.y);
    circles.push_back(circle2);
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    HDC hdc;
    PAINTSTRUCT ps;

    switch (msg) {
        case WM_LBUTTONDOWN:
            // Left click to set circle centers (maximum 2)
            if (clickPoints.size() < 2) {
                POINT pt;
                pt.x = LOWORD(lParam);
                pt.y = HIWORD(lParam);
                clickPoints.push_back(pt);

                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

        case WM_RBUTTONDOWN:
            // Right click to set touch point (only if 2 centers are set)
            if (clickPoints.size() == 2 && !touchPointSet) {
                touchPoint.x = LOWORD(lParam);
                touchPoint.y = HIWORD(lParam);
                touchPointSet = true;
                CalculateCircles();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            DrawAll(hdc);
            EndPaint(hwnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register window class
    const char* CLASS_NAME = "CircleConstructionClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Create window
    HWND hwnd = CreateWindowEx(
            0, CLASS_NAME, "Circle Construction Task",
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
            NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Show window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}
