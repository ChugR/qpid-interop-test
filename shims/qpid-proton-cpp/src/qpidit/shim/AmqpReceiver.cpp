/*
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */

#include "qpidit/shim/AmqpReceiver.hpp"

#include <iostream>
#include <json/json.h>
#include "proton/container.hpp"
#include "proton/decimal.hpp"
#include "proton/event.hpp"
#include "qpidit/QpidItErrors.hpp"

namespace qpidit
{
    namespace shim
    {

        AmqpReceiver::AmqpReceiver(const std::string& brokerUrl,
                                   const std::string& amqpType,
                                   uint32_t expected) :
                        _brokerUrl(brokerUrl),
                        _amqpType(amqpType),
                        _expected(expected),
                        _received(0UL),
                        _receivedValueList(Json::arrayValue)
        {}

        AmqpReceiver::~AmqpReceiver() {}

        Json::Value& AmqpReceiver::getReceivedValueList() {
            return _receivedValueList;
        }

        void AmqpReceiver::on_start(proton::event &e) {
            _receiver = e.container().open_receiver(_brokerUrl);
        }

        void AmqpReceiver::on_message(proton::event &e) {
            proton::message& msg = e.message();
            if (msg.id().get<uint64_t>() < _received) return; // ignore duplicate
            if (_received < _expected) {
                if (_amqpType.compare("null") == 0) {
                    checkMessageType(msg, proton::NULL_TYPE);
                    _receivedValueList.append("None");
                } else if (_amqpType.compare("boolean") == 0) {
                    checkMessageType(msg, proton::BOOLEAN);
                    _receivedValueList.append(msg.body().get<bool>() ? "True": "False");
                } else if (_amqpType.compare("ubyte") == 0) {
                    checkMessageType(msg, proton::UBYTE);
                    _receivedValueList.append(toHexStr<uint8_t>(msg.body().get<uint8_t>()));
                } else if (_amqpType.compare("ushort") == 0) {
                    checkMessageType(msg, proton::USHORT);
                    _receivedValueList.append(toHexStr<uint16_t>(msg.body().get<uint16_t>()));
                } else if (_amqpType.compare("uint") == 0) {
                    checkMessageType(msg, proton::UINT);
                    _receivedValueList.append(toHexStr<uint32_t>(msg.body().get<uint32_t>()));
                } else if (_amqpType.compare("ulong") == 0) {
                    checkMessageType(msg, proton::ULONG);
                    _receivedValueList.append(toHexStr<uint64_t>(msg.body().get<uint64_t>()));
                } else if (_amqpType.compare("byte") == 0) {
                    checkMessageType(msg, proton::BYTE);
                    _receivedValueList.append(toHexStr<int8_t>(msg.body().get<int8_t>()));
                } else if (_amqpType.compare("short") == 0) {
                    checkMessageType(msg, proton::SHORT);
                    _receivedValueList.append(toHexStr<int16_t>(msg.body().get<int16_t>()));
                } else if (_amqpType.compare("int") == 0) {
                    checkMessageType(msg, proton::INT);
                    _receivedValueList.append(toHexStr<int32_t>(msg.body().get<int32_t>()));
                } else if (_amqpType.compare("long") == 0) {
                    checkMessageType(msg, proton::LONG);
                    _receivedValueList.append(toHexStr<int64_t>(msg.body().get<int64_t>()));
                } else if (_amqpType.compare("float") == 0) {
                    checkMessageType(msg, proton::FLOAT);
                    float f = msg.body().get<float>();
                    _receivedValueList.append(toHexStr<uint32_t>(*((uint32_t*)&f), true));
                } else if (_amqpType.compare("double") == 0) {
                    checkMessageType(msg, proton::DOUBLE);
                    double d = msg.body().get<double>();
                    _receivedValueList.append(toHexStr<uint64_t>(*((uint64_t*)&d), true));
                } else if (_amqpType.compare("decimal32") == 0) {
                    checkMessageType(msg, proton::DECIMAL32);
                    _receivedValueList.append(byteArrayToHexStr(msg.body().get<proton::decimal32>()));
                } else if (_amqpType.compare("decimal64") == 0) {
                    checkMessageType(msg, proton::DECIMAL64);
                    _receivedValueList.append(byteArrayToHexStr(msg.body().get<proton::decimal64>()));
                } else if (_amqpType.compare("decimal128") == 0) {
                    checkMessageType(msg, proton::DECIMAL128);
                    _receivedValueList.append(byteArrayToHexStr(msg.body().get<proton::decimal128>()));
                } else if (_amqpType.compare("char") == 0) {
                    checkMessageType(msg, proton::CHAR);
                    wchar_t c = msg.body().get<wchar_t>();
                    std::stringstream oss;
                    if (c < 0x7f && std::iswprint(c)) {
                        oss << (char)c;
                    } else {
                        oss << "0x" << std::hex << c;
                    }
                    _receivedValueList.append(oss.str());
                } else if (_amqpType.compare("timestamp") == 0) {
                    checkMessageType(msg, proton::TIMESTAMP);
                    std::ostringstream oss;
                    oss << "0x" << std::hex << msg.body().get<proton::timestamp>().milliseconds();
                    _receivedValueList.append(oss.str());
                } else if (_amqpType.compare("uuid") == 0) {
                    checkMessageType(msg, proton::UUID);
                    std::ostringstream oss;
                    oss << msg.body().get<proton::uuid>();
                    _receivedValueList.append(oss.str());
                } else if (_amqpType.compare("binary") == 0) {
                    checkMessageType(msg, proton::BINARY);
                    _receivedValueList.append(msg.body().get<proton::amqp_binary>());
                } else if (_amqpType.compare("string") == 0) {
                    checkMessageType(msg, proton::STRING);
                    _receivedValueList.append(msg.body().get<proton::amqp_string>());
                } else if (_amqpType.compare("symbol") == 0) {
                    checkMessageType(msg, proton::SYMBOL);
                    _receivedValueList.append(msg.body().get<proton::amqp_symbol>());
                } else if (_amqpType.compare("list") == 0) {
                    checkMessageType(msg, proton::LIST);
                    Json::Value jsonList(Json::arrayValue);
                    _receivedValueList.append(getSequence(jsonList, msg.body()));
                } else if (_amqpType.compare("map") == 0) {
                    checkMessageType(msg, proton::MAP);
                    Json::Value jsonMap(Json::objectValue);
                    _receivedValueList.append(getMap(jsonMap, msg.body()));
                } else if (_amqpType.compare("array") == 0) {
                    throw qpidit::UnsupportedAmqpTypeError(_amqpType);
                } else {
                    throw qpidit::UnknownAmqpTypeError(_amqpType);
                }
            }
            _received++;
            if (_received >= _expected) {
                e.receiver().close();
                e.connection().close();
            }
        }

        // protected

        //static
        void AmqpReceiver::checkMessageType(const proton::message& msg, proton::type_id amqpType) {
            if (msg.body().type() != amqpType) {
                throw qpidit::IncorrectMessageBodyTypeError(amqpType, msg.body().type());
            }
        }

        //static
        Json::Value& AmqpReceiver::getMap(Json::Value& jsonMap, const proton::value& val) {
            std::map<proton::value, proton::value> msgMap;
            val.decode() >> proton::to_map(msgMap);
            for (std::map<proton::value, proton::value>::const_iterator i = msgMap.begin(); i != msgMap.end(); ++i) {
                switch (i->second.type()) {
                case proton::LIST:
                {
                    Json::Value jsonSubList(Json::arrayValue);
                    jsonMap[i->first.get<std::string>()] = getSequence(jsonSubList, i->second);
                    break;
                }
                case proton::MAP:
                {
                    Json::Value jsonSubMap(Json::objectValue);
                    jsonMap[i->first.get<std::string>()] = getMap(jsonSubMap, i->second);
                    break;
                }
                case proton::ARRAY:
                    break;
                case proton::STRING:
                    jsonMap[i->first.get<std::string>()] = Json::Value(i->second.get<std::string>());
                    break;
                default:
                    throw qpidit::IncorrectValueTypeError(i->second);
                }
            }
            return jsonMap;
        }

        //static
        Json::Value& AmqpReceiver::getSequence(Json::Value& jsonList, const proton::value& val) {
            std::vector<proton::value> msgList;
            val.decode() >> proton::to_sequence(msgList);
            for (std::vector<proton::value>::const_iterator i=msgList.begin(); i!=msgList.end(); ++i) {
                switch ((*i).type()) {
                case proton::LIST:
                {
                    Json::Value jsonSubList(Json::arrayValue);
                    jsonList.append(getSequence(jsonSubList, *i));
                    break;
                }
                case proton::MAP:
                {
                    Json::Value jsonSubMap(Json::objectValue);
                    jsonList.append(getMap(jsonSubMap, *i));
                    break;
                }
                case proton::ARRAY:
                    break;
                case proton::STRING:
                    jsonList.append(Json::Value((*i).get<std::string>()));
                    break;
                default:
                    throw qpidit::IncorrectValueTypeError(*i);
                }
            }
            return jsonList;
        }

        //static
        std::string AmqpReceiver::stringToHexStr(const std::string& str) {
            std::ostringstream oss;
            oss << "0x" << std::hex;
            for (std::string::const_iterator i=str.begin(); i!=str.end(); ++i) {
                oss << std::setw(2) << std::setfill('0') << ((int)*i & 0xff);
            }
            return oss.str();
        }

    } /* namespace shim */
} /* namespace qpidit */


/*
 * --- main ---
 * Args: 1: Broker address (ip-addr:port)
 *       2: Queue name
 *       3: AMQP type
 *       4: Expected number of test values to receive
 */

int main(int argc, char** argv) {
    // TODO: improve arg management a little...
    if (argc != 5) {
        throw qpidit::ArgumentError("Incorrect number of arguments");
    }

    std::ostringstream oss;
    oss << argv[1] << "/" << argv[2];

    try {
        qpidit::shim::AmqpReceiver receiver(oss.str(), argv[3], std::strtoul(argv[4], NULL, 0));
        proton::container(receiver).run();

        std::cout << argv[3] << std::endl;
        std::cout << receiver.getReceivedValueList();
    } catch (const std::exception& e) {
        std::cerr << "AmqpReceiver error: " << e.what() << std::endl;
        exit(-1);
    }
    exit(0);
}