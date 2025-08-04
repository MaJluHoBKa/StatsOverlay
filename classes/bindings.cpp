#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "json.hpp"
#include "MainStats.h"
#include "RatingStats.h"
#include "MasteryStats.h"
#include "OtherStats.h"
#include "VehicleStats.h"
#include "ApiController.h"

using json = nlohmann::json;
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
        .def("getDiffRating", &RatingStats::getDiffRating) // <--- Новый метод
        .def("getCalibBattles", &RatingStats::getCalibBattles)
        .def("getExpBattle", &RatingStats::getExpBattle)
        .def("getBattles", &RatingStats::getBattles)
        .def("getWins", &RatingStats::getWins)
        .def("getTotalDamage", &RatingStats::getTotalDamage)
        .def("getAvgDamage", &RatingStats::getAvgDamage)
        .def("getAvgExp", &RatingStats::getAvgExp)
        .def("getPercentWins", &RatingStats::getPercentWins);

    py::class_<MasteryData>(m, "MasteryData")
        .def(py::init<>())
        .def_readwrite("mastery", &MasteryData::mastery)
        .def_readwrite("mastery_1", &MasteryData::mastery_1)
        .def_readwrite("mastery_2", &MasteryData::mastery_2)
        .def_readwrite("mastery_3", &MasteryData::mastery_3)
        .def("__sub__", &MasteryData::operator-);

    py::class_<MasteryStats>(m, "MasteryStats")
        .def(py::init<>())
        .def("initialStats", &MasteryStats::initialStats)
        .def("updateStats", &MasteryStats::updateStats)
        .def("getFirstData", &MasteryStats::getFirstData, py::return_value_policy::reference_internal)
        .def("getCurrentData", &MasteryStats::getCurrentData, py::return_value_policy::reference_internal);

    py::class_<OtherData>(m, "OtherData")
        .def(py::init<>())
        .def_readwrite("battles", &OtherData::battles)
        .def_readwrite("hits", &OtherData::hits)
        .def_readwrite("shots", &OtherData::shots)
        .def_readwrite("survived", &OtherData::survived)
        .def_readwrite("frags", &OtherData::frags)
        .def_readwrite("receiverDamage", &OtherData::receiverDamage)
        .def_readwrite("totalDamage", &OtherData::totalDamage)
        .def_readwrite("lifeTime", &OtherData::lifeTime)
        .def("__sub__", &OtherData::operator-);

    py::class_<OtherStats>(m, "OtherStats")
        .def(py::init<>())
        .def("initialStats", &OtherStats::initialStats)
        .def("updateStats", &OtherStats::updateStats)
        .def("getPercentHits", &OtherStats::getPercentHits)
        .def("getPercentSurvived", &OtherStats::getPercentSurvived)
        .def("getLifeTime", &OtherStats::getLifeTime)
        .def("getDamageK", &OtherStats::getDamageK)
        .def("getFragsK", &OtherStats::getFragsK);

    py::class_<VehicleData>(m, "VehicleData")
        .def(py::init<>())
        .def_readwrite("id", &VehicleData::id)
        .def_readwrite("battles", &VehicleData::battles)
        .def_readwrite("totalDamage", &VehicleData::totalDamage)
        .def_readwrite("wins", &VehicleData::wins)
        .def("__sub__", &VehicleData::operator-);

    py::class_<VehicleStats>(m, "VehicleStats")
        .def(py::init<const std::string &>())
        .def("initialStats", &VehicleStats::initialStats)
        .def("updateStats", &VehicleStats::updateStats)
        .def("getUpdatedVehicle", [](const VehicleStats &vs) -> const VehicleData *
             {
                 return vs.getUpdatedVehicle(); // если nullptr — pybind11 вернёт None
             },
             py::return_value_policy::reference)
        .def("setNames", &VehicleStats::setNames)
        .def("getName", &VehicleStats::getName);

    py::class_<ApiController>(m, "ApiController")
        .def(py::init<>())
        .def("is_auth", &ApiController::is_auth)
        .def("login", &ApiController::login)
        .def("logout", &ApiController::logout)
        .def("prolongate", &ApiController::prolongate)
        .def("update_main_stats", &ApiController::update_main_stats)
        .def("update_rating_stats", &ApiController::update_rating_stats)
        .def("update_mastery_stats", &ApiController::update_mastery_stats)
        .def("update_other_stats", &ApiController::update_other_stats)
        .def("update_vehicles_stats", &ApiController::update_vehicles_stats)
        .def("get_vehicles_names", &ApiController::get_vehicles_names)
        .def("get_updated_vehicles", &ApiController::get_updated_vehicles,
             py::return_value_policy::reference)
        .def("getMainStats", &ApiController::getMainStats)
        .def("getRatingStats", &ApiController::getRatingStats)
        .def("getMasteryStats", &ApiController::getMasteryStats)
        .def("getOtherStats", &ApiController::getOtherStats)
        .def("getVehicleStats", &ApiController::getVehicleStats)
        .def("getNickname", &ApiController::getNickname)
        .def("getVehicleName", &ApiController::getVehicleName);
}
