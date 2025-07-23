#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "MainStats.h"
#include "RatingStats.h"

namespace py = pybind11;

PYBIND11_MODULE(stats, m)
{
    // StatsData (основной режим)
    py::class_<StatsData>(m, "StatsData")
        .def(py::init<>())
        .def_readwrite("credits", &StatsData::credits)
        .def_readwrite("gold", &StatsData::gold)
        .def_readwrite("exp_battle", &StatsData::exp_battle)
        .def_readwrite("exp_free", &StatsData::exp_free)
        .def_readwrite("battles", &StatsData::battles)
        .def_readwrite("wins", &StatsData::wins)
        .def_readwrite("losses", &StatsData::losses)
        .def_readwrite("totalDamage", &StatsData::totalDamage)
        .def("__sub__", &StatsData::operator-);

    py::class_<MainStats>(m, "MainStats")
        .def(py::init<>())
        .def("initialStats", &MainStats::initialStats)
        .def("updateStats", &MainStats::updateStats)
        .def("getGold", &MainStats::getGold)
        .def("getCredits", &MainStats::getCredits)
        .def("getExpBattle", &MainStats::getExpBattle)
        .def("getExpFree", &MainStats::getExpFree)
        .def("getBattles", &MainStats::getBattles)
        .def("getWins", &MainStats::getWins)
        .def("getLosses", &MainStats::getLosses)
        .def("getTotalDamage", &MainStats::getTotalDamage)
        .def("getAvgDamage", &MainStats::getAvgDamage)
        .def("getAvgExp", &MainStats::getAvgExp)
        .def("getPercentWins", &MainStats::getPercentWins);

    // RatingData (рейтинг)
    py::class_<RatingData>(m, "RatingData")
        .def(py::init<>())
        .def_readwrite("mm_rating", &RatingData::mm_rating)
        .def_readwrite("calib_battles", &RatingData::calib_battles)
        .def_readwrite("exp_battle", &RatingData::exp_battle)
        .def_readwrite("battles", &RatingData::battles)
        .def_readwrite("wins", &RatingData::wins)
        .def_readwrite("totalDamage", &RatingData::totalDamage)
        .def("__sub__", &RatingData::operator-);

    py::class_<RatingStats>(m, "RatingStats")
        .def(py::init<>())
        .def("initialStats", &RatingStats::initialStats)
        .def("updateStats", &RatingStats::updateStats)
        .def("getRating", &RatingStats::getRating)
        .def("getCalibBattles", &RatingStats::getCalibBattles)
        .def("getExpBattle", &RatingStats::getExpBattle)
        .def("getBattles", &RatingStats::getBattles)
        .def("getWins", &RatingStats::getWins)
        .def("getTotalDamage", &RatingStats::getTotalDamage)
        .def("getAvgDamage", &RatingStats::getAvgDamage)
        .def("getAvgExp", &RatingStats::getAvgExp)
        .def("getPercentWins", &RatingStats::getPercentWins);
}
