
#include "enum_utils.hpp"
#include <cassert>

typedef enum
{
    AMDT_PWR_CATEGORY_POWER,              /**< Instantaneous power */
    AMDT_PWR_CATEGORY_FREQUENCY,          /**< Frequency  */
    AMDT_PWR_CATEGORY_TEMPERATURE,        /**< Temperature in centigrade */
    AMDT_PWR_CATEGORY_VOLTAGE,            /**< Voltage */
    AMDT_PWR_CATEGORY_CURRENT,            /**< Current */
    AMDT_PWR_CATEGORY_PSTATE,             /**< P-State */
    AMDT_PWR_CATEGORY_CSTATES_RESIDENCY,  /**< C-State */
    AMDT_PWR_CATEGORY_TIME,               /**< Time */
    AMDT_PWR_CATEGORY_ENERGY,             /**< Energy consumed */
    AMDT_PWR_CATEGORY_CORRELATED_POWER,   /**< Energy consumed */
    AMDT_PWR_CATEGORY_CAC,                /**< CAC values  */
    AMDT_PWR_CATEGORY_CONTROLLER,         /**< Controller Limit and values */
    AMDT_PWR_CATEGORY_DPM,                /**< DPM State residency */
    AMDT_PWR_CATEGORY_CNT,                /**< Total category count */
} AMDTPwrCategory;


template<>
std::string EnumToString<AMDTPwrCategory>(AMDTPwrCategory e) {
    switch (e) {
    case AMDT_PWR_CATEGORY_POWER:
        return "AMDT_PWR_CATEGORY_POWER";
    case AMDT_PWR_CATEGORY_FREQUENCY:
        return "AMDT_PWR_CATEGORY_FREQUENCY";
    case AMDT_PWR_CATEGORY_TEMPERATURE:
        return "AMDT_PWR_CATEGORY_TEMPERATURE";
    case AMDT_PWR_CATEGORY_VOLTAGE:
        return "AMDT_PWR_CATEGORY_TEMPERATURE";
    case AMDT_PWR_CATEGORY_CURRENT:
        return "AMDT_PWR_CATEGORY_CURRENT";
    case AMDT_PWR_CATEGORY_PSTATE:
        return "AMDT_PWR_CATEGORY_PSTATE";
    case AMDT_PWR_CATEGORY_CSTATES_RESIDENCY:
        return "AMDT_PWR_CATEGORY_CSTATES_RESIDENCY";
    case AMDT_PWR_CATEGORY_TIME:
        return "AMDT_PWR_CATEGORY_TIME";
    case AMDT_PWR_CATEGORY_ENERGY:
        return "AMDT_PWR_CATEGORY_ENERGY";
    case AMDT_PWR_CATEGORY_CORRELATED_POWER:
        return "AMDT_PWR_CATEGORY_CORRELATED_POWER";
    case AMDT_PWR_CATEGORY_CAC:
        return "AMDT_PWR_CATEGORY_CAC";
    case AMDT_PWR_CATEGORY_CONTROLLER:
        return "AMDT_PWR_CATEGORY_CONTROLLER";
    case AMDT_PWR_CATEGORY_DPM:
        return "AMDT_PWR_CATEGORY_DPM";
    case AMDT_PWR_CATEGORY_CNT:
        return "AMDT_PWR_CATEGORY_CNT";
    }
    assert(false);
}


