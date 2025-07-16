#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "MainStats.h"

namespace py = pybind11;

PYBIND11_MODULE(stats, m)
{
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
        .def("getFirstData", &MainStats::getFirstData, py::return_value_policy::reference_internal)
        .def("getCurrentData", &MainStats::getCurrentData, py::return_value_policy::reference_internal)
        .def("getAvgDamage", &MainStats::getAvgDamage)
        .def("getAvgExp", &MainStats::getAvgExp)
        .def("getPercentWins", &MainStats::getPercentWins);
}