#pragma once

#include <cmath>
#include <algorithm>
#include <limits>

namespace sto
{
    // Можно работать в "нормированных" единицах, где c = 1,
    // или в обычных физических единицах, где c = 299792458.
    constexpr double SPEED_OF_LIGHT = 299'792'458.0;

    constexpr double eps = 1e-12;

    struct Event1D
    {
        double t; // время
        double x; // координата вдоль оси движения
    };

    struct Velocity1D
    {
        double v; // скорость вдоль оси x
    };

    struct LorentzResult1D
    {
        double t;
        double x;
    };

    inline double clamp(double x, double lo, double hi)
    {
        return std::max(lo, std::min(x, hi));
    }

    inline double beta(double v, double c = SPEED_OF_LIGHT)
    {
        return v / c;
    }

    inline double gamma_from_beta(double b)
    {
        const double bb = b * b;
        if (bb >= 1.0)
        {
            return std::numeric_limits<double>::infinity();
        }
        return 1.0 / std::sqrt(1.0 - bb);
    }

    inline double gamma(double v, double c = SPEED_OF_LIGHT)
    {
        return gamma_from_beta(beta(v, c));
    }

    inline bool is_speed_valid(double v, double c = SPEED_OF_LIGHT)
    {
        return std::abs(v) < c;
    }

    // Преобразование Лоренца из S в S'
    // S' движется вдоль +x со скоростью v относительно S.
    inline LorentzResult1D lorentz_transform(double t, double x, double v, double c = SPEED_OF_LIGHT)
    {
        const double b = beta(v, c);
        const double g = gamma_from_beta(b);

        LorentzResult1D r{};
        r.t = g * (t - (v * x) / (c * c));
        r.x = g * (x - v * t);
        return r;
    }

    inline LorentzResult1D inverse_lorentz_transform(double tPrime, double xPrime, double v, double c = SPEED_OF_LIGHT)
    {
        // Обратное преобразование — то же самое, но с -v
        return lorentz_transform(tPrime, xPrime, -v, c);
    }

    // Инвариант интервала:
    // s^2 = c^2 t^2 - x^2
    // Для пары событий:
    // s^2 = c^2 (Δt)^2 - (Δx)^2
    inline double interval_squared(double dt, double dx, double c = SPEED_OF_LIGHT)
    {
        return c * c * dt * dt - dx * dx;
    }

    inline double interval_squared(const Event1D& a, const Event1D& b, double c = SPEED_OF_LIGHT)
    {
        return interval_squared(b.t - a.t, b.x - a.x, c);
    }

    // Собственное время между двумя событиями.
    // Имеет смысл для тимelike-интервала: c^2 dt^2 > dx^2.
    inline double proper_time(double dt, double dx, double c = SPEED_OF_LIGHT)
    {
        const double s2 = interval_squared(dt, dx, c);
        if (s2 <= 0.0)
        {
            return 0.0; // для светоподобных и пространственноподобных интервалов не определено как обычное время
        }
        return std::sqrt(s2) / c;
    }

    inline double proper_time(const Event1D& a, const Event1D& b, double c = SPEED_OF_LIGHT)
    {
        return proper_time(b.t - a.t, b.x - a.x, c);
    }

    // Замедление времени:
    // Δt = γ Δτ
    // где Δτ — собственное время движущихся часов
    inline double time_dilation(double proper_dt, double v, double c = SPEED_OF_LIGHT)
    {
        return gamma(v, c) * proper_dt;
    }

    // Обратная форма:
    // Δτ = Δt / γ
    inline double proper_time_from_lab_time(double lab_dt, double v, double c = SPEED_OF_LIGHT)
    {
        return lab_dt / gamma(v, c);
    }

    // Сокращение длины:
    // L = L0 / γ
    // где L0 — длина покоящегося объекта
    inline double length_contraction(double rest_length, double v, double c = SPEED_OF_LIGHT)
    {
        return rest_length / gamma(v, c);
    }

    // Относительность одновременности.
    // Если в одной системе два события одновременны: Δt = 0,
    // то в движущейся системе:
    // Δt' = - γ v Δx / c^2
    inline double simultaneity_shift(double dx, double v, double c = SPEED_OF_LIGHT)
    {
        return -gamma(v, c) * v * dx / (c * c);
    }

    // Более общая форма:
    // Δt' = γ (Δt - v Δx / c^2)
    inline double time_difference_transformed(double dt, double dx, double v, double c = SPEED_OF_LIGHT)
    {
        return gamma(v, c) * (dt - (v * dx) / (c * c));
    }

    // Более общая форма:
    // Δx' = γ (Δx - v Δt)
    inline double space_difference_transformed(double dt, double dx, double v, double c = SPEED_OF_LIGHT)
    {
        return gamma(v, c) * (dx - v * dt);
    }

    // Сложение скоростей в 1D:
    // u' = (u + v) / (1 + uv/c^2)
    // Здесь u — скорость объекта в S,
    // v — скорость системы S' относительно S.
    inline double velocity_addition_1d(double u, double v, double c = SPEED_OF_LIGHT)
    {
        const double denom = 1.0 + (u * v) / (c * c);
        if (std::abs(denom) < eps)
        {
            return std::copysign(c, u + v);
        }
        return (u + v) / denom;
    }

    inline double velocity_subtraction_1d(double u, double v, double c = SPEED_OF_LIGHT)
    {
        // Скорость объекта в системе, движущейся со скоростью v
        // относительно исходной системы:
        // (u - v) / (1 - uv/c^2)
        const double denom = 1.0 - (u * v) / (c * c);
        if (std::abs(denom) < eps)
        {
            return std::copysign(c, u - v);
        }
        return (u - v) / denom;
    }

    // Энергия покоя
    inline double rest_energy(double mass, double c = SPEED_OF_LIGHT)
    {
        return mass * c * c;
    }

    // Полная релятивистская энергия
    inline double total_energy(double mass, double v, double c = SPEED_OF_LIGHT)
    {
        return gamma(v, c) * mass * c * c;
    }

    // Релятивистский импульс
    inline double momentum(double mass, double v, double c = SPEED_OF_LIGHT)
    {
        return gamma(v, c) * mass * v;
    }

    // Связь энергии и импульса:
    // E^2 = (pc)^2 + (mc^2)^2
    inline double energy_from_momentum(double p, double mass, double c = SPEED_OF_LIGHT)
    {
        return std::sqrt((p * c) * (p * c) + std::pow(mass * c * c, 2));
    }

    // Доплеровский сдвиг для прямолинейного движения вдоль луча.
    // approach = true  -> источник/наблюдатель сближаются
    // approach = false -> удаляются
    //
    // f = f0 * sqrt((1 + β)/(1 - β))  при сближении
    // f = f0 * sqrt((1 - β)/(1 + β))  при удалении
    inline double doppler_frequency(double f0, double v, bool approach, double c = SPEED_OF_LIGHT)
    {
        const double b = clamp(beta(v, c), -0.999999999999, 0.999999999999);

        if (approach)
        {
            return f0 * std::sqrt((1.0 + b) / (1.0 - b));
        }
        return f0 * std::sqrt((1.0 - b) / (1.0 + b));
    }

    // Приближённо для визуализации цвета:
    // 0.0 -> краснее, 1.0 -> синее
    inline double doppler_color_shift(double v, double c = SPEED_OF_LIGHT)
    {
        const double b = clamp(std::abs(beta(v, c)), 0.0, 0.999999999999);
        return std::sqrt((1.0 + b) / (1.0 - b));
    }

    // Удобная проверка, что преобразование не даёт скорость света
    inline bool velocity_within_light_speed(double v, double c = SPEED_OF_LIGHT)
    {
        return std::abs(v) < c;
    }

    // Параметры линии мира, полезно для анимации
    struct Worldline1D
    {
        double x0;   // начальная координата
        double v;    // скорость
        double t0;   // стартовое время
    };

    inline double worldline_x(const Worldline1D& wl, double t)
    {
        return wl.x0 + wl.v * (t - wl.t0);
    }

    inline Event1D event_on_worldline(const Worldline1D& wl, double t)
    {
        return Event1D{t, worldline_x(wl, t)};
    }

    // "Лабораторное" отображение объекта в момент t
    // (упрощённо, для визуализации, если объект движется равномерно)
    inline double contracted_length_for_render(double rest_length, double v, double c = SPEED_OF_LIGHT)
    {
        return length_contraction(rest_length, v, c);
    }

    // Нормировка скорости в диапазон [0,1] для UI-ползунка
    inline double normalize_speed(double v, double c = SPEED_OF_LIGHT)
    {
        return clamp(std::abs(v) / c, 0.0, 0.999999999999);
    }

    inline double denormalize_speed(double slider, double c = SPEED_OF_LIGHT)
    {
        const double s = clamp(slider, 0.0, 0.999999999999);
        return s * c;
    }
}