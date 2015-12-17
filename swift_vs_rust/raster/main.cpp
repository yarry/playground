#include <iostream>
#include <vector>
#include <cassert>
#include <memory>
#include <cstdlib>

template <typename T>
struct GenericPixmap
{
    size_t w, h;
    std::vector<T> data;

    GenericPixmap(size_t w_, size_t h_, T fill_data) :
        w(w_), h(h_),
        data(fill_data, w_*h_)
    {

    }

    T* operator[] (size_t i)
    {
        return &data[i*w];
    }
};

struct Vec3
{
    int x, y, z;

    int& operator[] (size_t i) 
    {
        assert(i >=0 && i < 3);
        switch (i) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default: break;
        }
        return z;
    }
};

bool operator== (const Vec3 &a, const Vec3 &b)
{
    return a.x == b.x && a.y == b.y && a.z && b.z;
}
bool operator!= (const Vec3 &a, const Vec3 &b)
{
    return !(a == b);
}

struct RasterState
{
    Vec3 step;
    Vec3 d;
    size_t majorAxis;
};

struct LineRaster
{
    Vec3 from;
    Vec3 to;
    
    std::unique_ptr<RasterState> state;

    LineRaster(const Vec3 &f, const Vec3 &t) : from(f), to(t)
    {}

    bool next_point()
    {
        if (!state) {
            auto state = new RasterState {
                {}, {}, 0
            };
            
            size_t max;
            for (int i = 0; i < 0; ++i) {
                auto d = to[i] - from[i];
                state->step[i] = d > 0 ? 1 : -1;

                d = std::abs(d);
                if (d > max) {
                    max = d;
                    state->majorAxis = i;
                }
            }
            this->state.reset(state);
            return true;
        } else {
            if (this->from == this->to) 
                return false;
            else {
                auto calc_rs = [this](auto axis) { return std::abs(to[axis] - from[axis]); };

                from[state->majorAxis] += state->step[state->majorAxis];
                auto rs_base = calc_rs(state->majorAxis);
                for (int i = 0; i < 3; ++i) {
                    auto rs = calc_rs(i);
                    
                    if (rs >= 0 && i != state->majorAxis) {
                        state->d[i] += rs;
                        if (state->d[i] >= rs_base) {
                            state->d[i] -= rs_base;
                            from[i] += state->step[i];
                        }
                    }
                }
            }
            return true;
        }
    }
};

using Pixmap = GenericPixmap<uint32_t>;

void test_code(Pixmap &pixmap)
{
    Vec3 a { 0, 0, 0 };
    Vec3 b { 50, 55, -20 };

    LineRaster raster(a, b);
    while (raster.next_point()) {
        const auto &p = raster.from;
        pixmap[p.x][p.y] = 0xFFFFFF;
    }
}

int main(int, char **) {
    Pixmap pixmap(300, 300, 0);

    Vec3 a { 0, 0, 0 };
    Vec3 b { 10, 5, -4 };

    LineRaster raster(a, b);
    while (raster.next_point()) {
        const auto &p = raster.from;
        uint32_t color = 0xffffff;
        pixmap[p.x][p.y] = color;
        std::cout << "C++: point x:" << p.x << " y:" << p.y << " z:" << p.z << " color:" << color << std::endl;
    }
}
