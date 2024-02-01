#include <array>
#include <raylib.h>
#include <unordered_map>
#include <vector>

float offsetX = 0, offsetY = 0;
float zoom = 1;

template <typename Type>
struct Point {
	Type x, y;
	float dot(const Point& other) const {
		return x * other.x + y * other.y;
	}
	Point operator+(const Point& other) const {
		return {x + other.x, y + other.y};
	}
	Point& operator+=(const Point& other) {
		x += other.x;
		y += other.y;
		return *this;
	}
	Point operator*(const Point& other) const {
		return {x * other.x, y * other.y};
	}
	Point& operator*=(const Point& other) {
		x *= other.x;
		y *= other.y;
		return *this;
	}
	template <typename T>
	Point operator+(const T& other) const {
		return {x + other, y + other};
	}
	template <typename T>
	Point& operator+=(const T& other) {
		x += other;
		y += other;
		return *this;
	}
	template <typename T>
	Point operator*(const T& other) const {
		return {x * other, y * other};
	}
	template <typename T>
	Point& operator*=(const T& other) {
		x *= other;
		y *= other;
		return *this;
	}
	bool operator==(const Point& p2) const {
		return x == p2.x && y == p2.y;
	}
};
template <>
struct std::hash<Point<int>> {
	size_t operator()(const Point<int>& k) const noexcept {
		size_t seed = 0;
		seed ^= std::hash<size_t>()(k.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= std::hash<size_t>()(k.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		return seed;
	}
};

float GetRandomValueFloat(float vmin, float vmax) {
	return vmin + static_cast<float>(GetRandomValue(0, INT_MAX)) / (INT_MAX / (vmax - vmin));
}

std::vector<Point<int>> RandomWalkGrid(size_t numIterations, Point<int> startPosition) {
	std::vector<Point<int>> points;
	points.reserve(numIterations);

	points.push_back(startPosition);
	for (size_t i = 1; i < numIterations; i++) {
		auto p = points[i - 1];

		constexpr Point<int> dirs[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

		// select random dir and radius
		const int dir = GetRandomValue(0, 3);
		const int r = GetRandomValue(1, 2);
		p += dirs[dir] * r;

		points.push_back(p);
	}

	return points;
}
std::vector<Point<int>> RandomWalkGridAvoidItself(size_t numIterations, Point<int> startPosition) {
	std::vector<Point<int>> points;
	points.reserve(numIterations);
	std::unordered_map<Point<int>, bool> visited;

	points.push_back(startPosition);
	visited[startPosition] = true;
	for (size_t i = 1; i < numIterations; i++) {
		auto p = points[i - 1];

		const int r = GetRandomValue(1, 2);
		// check if visited
		constexpr Point<int> dirs[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
		std::vector<Point<int>> availableDirs;
		for (int j = 0; j < 4; j++) {
			if (visited[p + dirs[j] * r]) continue;
			availableDirs.push_back(dirs[j] * r);
		}

		// if none free, exit
		if (availableDirs.empty()) break;

		// select random from available directions
		const int dir = GetRandomValue(0, static_cast<int>(availableDirs.size() - 1));

		p += availableDirs[dir];
		visited[p] = true;

		points.push_back(p);
	}

	return points;
}
std::vector<Point<float>> RandomWalkFloat(size_t numIterations, Point<float> startPosition) {
	std::vector<Point<float>> points;
	points.reserve(numIterations);

	points.push_back(startPosition);
	for (size_t i = 1; i < numIterations; i++) {
		auto p = points[i - 1];

		// go in same or -same direction as prev point with some rand
		Point<float> offset;
		if (i >= 2) {
			const auto p2 = points[i - 2];
			Point<float> dir = {p.x - p2.x, p.y - p2.y};
			dir *= 1.f / sqrtf(dir.dot(dir));
			float angle = GetRandomValueFloat(0, 2 * PI);
			offset = {cosf(angle), sinf(angle)};
			while (abs(dir.dot(offset)) < 0.98f) {
				angle = GetRandomValueFloat(0, 2 * PI);
				offset = {cosf(angle), sinf(angle)};
			}
		}
		else {
			const float dir = GetRandomValueFloat(0, 2 * PI);
			offset = {cosf(dir), sinf(dir)};
		}

		offset *= GetRandomValueFloat(2.5f, 7.5f);
		p += offset;

		points.push_back(p);
	}

	return points;
}

void WorldToScreen(float* x, float* y) {
	*x = (*x + offsetX) * zoom;
	*x += static_cast<float>(GetScreenWidth()) / 2.f;
	*y = (*y + offsetY) * zoom;
	*y += static_cast<float>(GetScreenHeight()) / 2.f;
}
void ScreenToWorld(float* x, float* y) {
	*x -= static_cast<float>(GetScreenWidth()) / 2.f;
	*x = *x / zoom - offsetX;
	*y -= static_cast<float>(GetScreenHeight()) / 2.f;
	*y = *y / zoom - offsetY;
}

template <typename T>
void DrawPointList(const std::vector<Point<T>>& points, Color color) {
	if (points.size() < 2) return;

	for (size_t i = 1; i < points.size(); i++) {
		const auto p1 = points[i - 1];
		const auto p2 = points[i];

		float startX = static_cast<float>(p1.x), startY = static_cast<float>(p1.y);
		WorldToScreen(&startX, &startY);
		float endX = static_cast<float>(p2.x), endY = static_cast<float>(p2.y);
		WorldToScreen(&endX, &endY);

		DrawLine(static_cast<int>(startX), static_cast<int>(startY),
			static_cast<int>(endX), static_cast<int>(endY), color);
	}
}

void ProcessInput() {
	constexpr float speed = 240.f;
	constexpr float zoomSpeed = 1.f;
	constexpr float zoomMin = 0.1f;

	if (IsKeyDown(KEY_W)) offsetY += speed / zoom * GetFrameTime();
	if (IsKeyDown(KEY_S)) offsetY -= speed / zoom * GetFrameTime();
	if (IsKeyDown(KEY_A)) offsetX += speed / zoom * GetFrameTime();
	if (IsKeyDown(KEY_D)) offsetX -= speed / zoom * GetFrameTime();

	float scrollDiff = GetMouseWheelMove();
	if (IsKeyDown(KEY_LEFT_SHIFT)) scrollDiff *= 0.05f;
	if (scrollDiff != 0.f) {
		float oldMouseWorldX = static_cast<float>(GetMouseX());
		float oldMouseWorldY = static_cast<float>(GetMouseY());
		ScreenToWorld(&oldMouseWorldX, &oldMouseWorldY);

		zoom += zoomSpeed * scrollDiff * sqrtf(zoom / 2.f);
		zoom = zoom < zoomMin ? zoomMin : zoom;

		float newMouseWorldX = static_cast<float>(GetMouseX());
		float newMouseWorldY = static_cast<float>(GetMouseY());
		ScreenToWorld(&newMouseWorldX, &newMouseWorldY);
		offsetX += newMouseWorldX - oldMouseWorldX;
		offsetY += newMouseWorldY - oldMouseWorldY;
	}
}

int main() {
	constexpr int windowWidth = 1000, windowHeight = 800;
	SetWindowState(FLAG_WINDOW_ALWAYS_RUN);
	InitWindow(windowWidth, windowHeight, "Random walk");
	SetTargetFPS(120);

	constexpr int numWalks = 7;
	constexpr size_t numIters = 25000;

	std::vector<std::vector<Point<float>>> points;
	for (size_t i = 0; i < numWalks; i++) {
		points.push_back(RandomWalkFloat(numIters,
			{GetRandomValueFloat(-100, 100), GetRandomValueFloat(-100, 100)}));
	}

	constexpr std::array<Color, 10> colors = {
		RED, GREEN, BLUE, ORANGE, YELLOW, MAROON, DARKPURPLE, BEIGE, RAYWHITE, DARKBLUE
	};

	while (!WindowShouldClose()) {
		ProcessInput();

		if (IsKeyReleased(KEY_R)) {
			points.clear();
			for (size_t i = 0; i < numWalks; i++) {
				points.push_back(RandomWalkFloat(numIters,
					{GetRandomValueFloat(-500, 500), GetRandomValueFloat(-500, 500)}));
			}
		}

		BeginDrawing();
		ClearBackground(BLACK);
		for (size_t i = 0; i < points.size(); i++) {
			DrawPointList(points[i], colors[i % colors.size()]);
		}
		EndDrawing();
		static int windowWidthOld = GetScreenWidth();
		static int windowHeightOld = GetScreenHeight();
		if (IsKeyReleased(KEY_F11)) {
			if (IsWindowFullscreen()) {
				ToggleFullscreen();
				SetWindowSize(windowWidthOld, windowHeightOld);
			}
			else {
				SetWindowSize(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor()));
				ToggleFullscreen();
			}
		}
	}

	CloseWindow();

	return 0;
}
