/****************************************************************************
 * Copyright (c) 2019 Tomi Lähteenmäki <lihis@lihis.net>                    *
 *                                                                          *
 * This program is free software; you can redistribute it and/or modify     *
 * it under the terms of the GNU General Public License as published by     *
 * the Free Software Foundation; either version 2 of the License, or        *
 * (at your option) any later version.                                      *
 *                                                                          *
 * This program is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 * GNU General Public License for more details.                             *
 *                                                                          *
 * You should have received a copy of the GNU General Public License        *
 * along with this program; if not, write to the Free Software              *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,               *
 * MA 02110-1301, USA.                                                      *
 ****************************************************************************/

/**
 * @file PluginDefs.h
 * @brief Shared header for plugin and application
 * @details Definition of data transported via WebSocket.
 * @author Tomi Lähteenmäki
 * @license This project is licensed under GNU General Public License, Version 2
 */

#ifndef ETS2_JOB_LOGGER_PLUGINDEFS_H
#define ETS2_JOB_LOGGER_PLUGINDEFS_H

#include <jobplugin/Version.h>
#include <string>
#include <msgpack.hpp>
#ifndef PLUGIN_INTERNAL
#include <json/value.h>
#endif

#define WEBSOCK_PORT 20210

enum class PacketType {
    Unknown = 0,
    Version = 1,
    Job = 2,
    Truck = 3,
    CargoDamage = 4,
    Fine = 5
};
MSGPACK_ADD_ENUM(PacketType)

enum class Game {
    Unknown = 0,
    ETS2 = 1,
    ATS = 2
};
MSGPACK_ADD_ENUM(Game)

enum class Fine {
    Unknown = 0,
    Crash = 1,
    AvoidSleeping = 2,
    WrongWay = 3,
    SpeedingCamera = 4,
    Speeding = 5,
    NoLights = 6,
    RedSignal = 7,
    Sleeping = 8,
    AvoidWeighing = 9,
    IllegalTrailer = 10,
    Generic = 11
};
MSGPACK_ADD_ENUM(Fine);

struct version_t {
    explicit version_t(uint8_t major = 0, uint8_t minor = 0, uint8_t patch = 0) {
        this->major = 0;
        this->minor = 0;
        this->patch = 0;
    }

    uint8_t major;
    uint8_t minor;
    uint8_t patch;

    MSGPACK_DEFINE(major, minor, patch);
};

struct fine_t {
    fine_t() : type(Fine::Unknown), amount(0) {
    }

    Fine type;
    int64_t amount;

    MSGPACK_DEFINE(type, amount);

#ifndef PLUGIN_INTERNAL
    void Serialize(Json::Value &root) const {
        root["type"] = (uint8_t)type;
        root["amount"] = Json::Value::Int64(amount);
    }
#endif
};

struct id_name_t {
    id_name_t() {
        id = "";
        name = "";
    }

    std::string id;
    std::string name;

    MSGPACK_DEFINE(id, name);

#ifndef PLUGIN_INTERNAL
    void Serialize(Json::Value &root) const {
        root["id"] = id;
        root["name"] = name;
    }
#endif
};
typedef struct id_name_t city_t;
typedef struct id_name_t company_t;

struct source_destination_t {
    city_t city;
    company_t company;

    MSGPACK_DEFINE(city, company);

#ifndef PLUGIN_INTERNAL
    void Serialize(Json::Value &root, bool is_special) const {
        root["city"] = Json::Value();
        city.Serialize(root["city"]);

        if (!is_special) {
            root["company"] = Json::Value();
            company.Serialize(root["company"]);
        }
    }
#endif
};
typedef source_destination_t source_t;
typedef source_destination_t destination_t;

struct truck_t {
    truck_t() : odometer(-1.f), fuel(-1.f), speed(0.f), x(0.f), y(0.f), z(0.f), heading(0.f) {
    }

    float odometer;
    float fuel;
    float speed;
    double x;
    double y;
    double z;
    float heading;

    MSGPACK_DEFINE(odometer, fuel, speed, x, y, z, heading);

#ifndef PLUGIN_INTERNAL
    void Serialize(Json::Value &root) const {
        root["speed"] = speed;
        root["x"] = x;
        root["y"] = y;
        root["z"] = z;
        root["heading"] = heading;
    }
#endif
};

enum JobStatus {
    FreeAsWind = 0,
    OnJob = 1,
    Cancelled = 2,
    Delivered = 3
};
MSGPACK_ADD_ENUM(JobStatus);

struct job_t {
    explicit job_t(const Game &game = Game::Unknown) {
        this->game = game;
        status = JobStatus::FreeAsWind;
        isSpecial = false;
        fuelConsumed = 0.f;
        maxSpeed = 0.f;
        income = 0;
        trailer = {};
        cargo = {};
        source = {};
        destination = {};
    }

    Game game;
    JobStatus status;
    bool isSpecial;

    struct distance_t {
        float driven;
        uint32_t planned;

        MSGPACK_DEFINE(driven, planned);
    } distance = {};
    float fuelConsumed;
    float maxSpeed;

    uint64_t income;

    struct trailer_t {
        trailer_t() {
            connected = false;
        }

        bool connected;
        MSGPACK_DEFINE(connected)
    } trailer;

    struct cargo_t {
        cargo_t() {
            id = "";
            name = "-";
            mass = 0.f;
            damage = 0.f;
        }

        std::string id;
        std::string name;
        float mass;
        float damage;
        MSGPACK_DEFINE(name, id, mass, damage)
    } cargo;

    source_t source;
    destination_t destination;

    MSGPACK_DEFINE(game, status, isSpecial, distance, fuelConsumed, maxSpeed, income, trailer, cargo, source, destination);

#ifndef PLUGIN_INTERNAL
    void Serialize(Json::Value &root) const {
        std::string game_name;

        switch (game) {
            case Game::ETS2:
                game_name = "ets2";
                break;
            case Game::ATS:
                game_name = "ats";
                break;
            case Game::Unknown:
            default:
                break;
        }

        root["game"] = game_name;
        root["status"] = (uint8_t)status;
        root["isSpecial"] = isSpecial;
        root["income"] = Json::Value::UInt64(income);
        root["maxSpeed"] = maxSpeed;
        root["fuelConsumed"] = fuelConsumed;

        Json::Value distanceObj;
        distanceObj["driven"] = distance.driven;
        distanceObj["planned"] = distance.planned;
        root["distance"] = distanceObj;

        Json::Value cargoObj;
        cargoObj["name"] = cargo.name;
        cargoObj["id"] = cargo.id;
        cargoObj["mass"] = cargo.mass;
        cargoObj["damage"] = cargo.damage;
        root["cargo"] = cargoObj;

        root["source"] = Json::Value();
        source.Serialize(root["source"], isSpecial);

        root["destination"] = Json::Value();
        destination.Serialize(root["destination"], isSpecial);
    }
#endif
};

#endif //ETS2_JOB_LOGGER_PLUGINDEFS_H
