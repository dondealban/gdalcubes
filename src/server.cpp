/*
   Copyright 2018 Marius Appel <marius.appel@uni-muenster.de>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "server.h"
#include <cpprest/filestream.h>
#include <cpprest/rawptrstream.h>
#include <boost/program_options.hpp>
#include <condition_variable>
#include "build_info.h"
#include "cube_factory.h"
#include "image_collection.h"
#include "utils.h"
/**
GET  /version
POST /file (name query, body file)
POST /cube (json process descr), return cube_id
GET /cube/{cube_id}
POST /cube/{cube_id}/{chunk_id}/start
GET /cube/{cube_id}/{chunk_id}/status status= "notrequested", "submitted" "queued" "running" "canceled" "finished" "error"
GET /cube/{cube_id}/{chunk_id}/download

**/

server_chunk_cache* server_chunk_cache::_instance = nullptr;
std::mutex server_chunk_cache::_singleton_mutex;

void gdalcubes_server::handle_get(web::http::http_request req) {
    std::vector<std::string> path = web::uri::split_path(web::uri::decode(req.relative_uri().path()));
    std::map<std::string, std::string> query_pars = web::uri::split_query(web::uri::decode(req.relative_uri().query()));
    //    std::for_each(path.begin(), path.end(), [](std::string s) { std::cout << s << std::endl; });
    //    std::for_each(query_pars.begin(), query_pars.end(), [](std::pair<std::string, std::string> s) { std::cout << s.first << ":" << s.second << std::endl; });
    if (!path.empty()) {
        if (path[0] == "version") {
            std::cout << "GET /version" << std::endl;
            std::string version = "gdalcubes_server " + std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR) + "." + std::to_string(VERSION_PATCH);
            req.reply(web::http::status_codes::OK, version.c_str(), "text/plain");
        } else if (path[0] == "cube") {
            if (path.size() == 2) {
                uint32_t cube_id = std::stoi(path[1]);
                std::cout << "GET /cube/" << cube_id << std::endl;

                if (_cubestore.find(cube_id) == _cubestore.end()) {
                    req.reply(web::http::status_codes::NotFound, "ERROR in /GET /cube/{cube_id}: cube with given id is not available.", "text/plain");
                } else {
                    req.reply(web::http::status_codes::OK, _cubestore[cube_id]->make_constructible_json().dump(2).c_str(),
                              "application/json");
                }
            } else if (path.size() == 4) {
                uint32_t cube_id = std::stoi(path[1]);
                uint32_t chunk_id = std::stoi(path[2]);
                
                //debug 
                if (chunk_id == 48) {
                    std::cout << "BREAK";
                }
                
               // uint32_t chunk_id = 0; // debug
                std::string cmd = path[3];
                if (cmd == "download") {
                    std::cout << "GET /cube/" << cube_id << "/" << chunk_id << "/download" << std::endl;
                    if (_cubestore.find(cube_id) == _cubestore.end()) {
                        req.reply(web::http::status_codes::NotFound, "ERROR in /GET /cube/{cube_id}/{chunk_id}/download: cube is not available", "text/plain");
                    } else if (chunk_id >= _cubestore[cube_id]->count_chunks()) {
                        req.reply(web::http::status_codes::NotFound, "ERROR in /GET /cube/{cube_id}/{chunk_id}/download: invalid chunk_id given", "text/plain");
                    }
                    // if not in queue, executing, or finished, return 404
                    else if (!server_chunk_cache::instance()->has(std::make_pair(cube_id, chunk_id)) &&
                             _chunk_read_requests_set.find(std::make_pair(cube_id, chunk_id)) == _chunk_read_requests_set.end() &&
                             _chunk_read_executing.find(std::make_pair(cube_id, chunk_id)) == _chunk_read_executing.end()) {
                        req.reply(web::http::status_codes::NotFound, "ERROR in /GET /cube/{cube_id}/{chunk_id}/download: chunk read has not been requested yet", "text/plain");
                    } else {
                        while (!server_chunk_cache::instance()->has(std::make_pair(cube_id, chunk_id))) {
                            // req.reply(web::http::status_codes::NotFound, "ERROR in /GET /cube/{cube_id}/{chunk_id}/download: chunk is not available.", "text/plain");

                            std::unique_lock<std::mutex> lck(_chunk_cond[std::make_pair(cube_id, chunk_id)].second);
                            _chunk_cond[std::make_pair(cube_id, chunk_id)].first.wait(lck);
                        }
                        std::shared_ptr<chunk_data> dat = server_chunk_cache::instance()->get(std::make_pair(cube_id, chunk_id));
                        std::cout << "chunk " << chunk_id << ": " << (double)server_chunk_cache::instance()->total_size_bytes() / (double)(1024*1024) << std::endl;
                        if (dat->empty())
                            std::cout << "chunk " << chunk_id << " is empty" << std::endl;


                        uint8_t* rawdata = (uint8_t*)malloc(4 * sizeof(uint32_t) + dat->total_size_bytes());
                        memcpy((void*)rawdata,(void*)(dat->size().data()),4 * sizeof(uint32_t));
                        if (!dat->empty()) {
                            memcpy(rawdata + 4 * sizeof(uint32_t), dat->buf(), dat->total_size_bytes());
                        }

                        concurrency::streams::basic_istream<uint8_t> is = concurrency::streams::rawptr_stream<uint8_t>::open_istream(rawdata, 4 * sizeof(uint32_t) + dat->total_size_bytes() );
                        if (!is.is_open()){
                            std::cout << "input stream is not open" << std::endl;
                        }


                        req.reply(web::http::status_codes::OK, is,  4 * sizeof(uint32_t) + dat->total_size_bytes(), "application/octet-stream").then([&is, &rawdata](){
                            is.close();
                            free(rawdata);});


//                        std::shared_ptr<std::vector<unsigned char>> rawdata = std::make_shared<std::vector<unsigned char>>();
//                        rawdata->resize(4 * sizeof(uint32_t) + dat->total_size_bytes());
//                        std::copy((unsigned char*)(dat->size().data()), (unsigned char*)(dat->size().data()) + (sizeof(uint32_t) * 4), (unsigned char*)(rawdata->data()));
//                        if (!dat->empty())
//                            std::copy((unsigned char*)(dat->buf()), (unsigned char*)(dat->buf() + dat->total_size_bytes()), rawdata->begin() + (sizeof(uint32_t) * 4));
//
//                        if (query_pars.find("clean") != query_pars.end() && query_pars["clean"] == "true") {
//                            server_chunk_cache::instance()->remove(std::make_pair(cube_id, chunk_id));
//                        }
//                        web::http::http_response res;
//                        res.set_body(*rawdata);
//                        res.set_status_code(web::http::status_codes::OK);
//                        req.reply(res);
                    }

                } else if (cmd == "status") {
                    std::cout << "GET /cube/" << cube_id << "/" << chunk_id << "/status" << std::endl;
                    if (_cubestore.find(cube_id) == _cubestore.end()) {
                        req.reply(web::http::status_codes::NotFound, "ERROR in /GET /cube/{cube_id}/{chunk_id}/status: cube is not available", "text/plain");
                    } else if (chunk_id >= _cubestore[cube_id]->count_chunks()) {
                        req.reply(web::http::status_codes::NotFound, "ERROR in /GET /cube/{cube_id}/{chunk_id}/status: invalid chunk_id given", "text/plain");
                    }

                    if (server_chunk_cache::instance()->has(std::make_pair(cube_id, chunk_id))) {
                        req.reply(web::http::status_codes::OK, "finished", "text/plain");
                    } else if (_chunk_read_executing.find(std::make_pair(cube_id, chunk_id)) !=
                               _chunk_read_executing.end()) {
                        req.reply(web::http::status_codes::OK, "running", "text/plain");
                    } else if (_chunk_read_requests_set.find(std::make_pair(cube_id, chunk_id)) != _chunk_read_requests_set.end()) {
                        req.reply(web::http::status_codes::OK, "queued", "text/plain");
                    } else {
                        req.reply(web::http::status_codes::OK, "notrequested", "text/plain");
                    }

                } else {
                    req.reply(web::http::status_codes::NotFound);
                }
            } else {
                req.reply(web::http::status_codes::NotFound);
            }
        }

        else {
            req.reply(web::http::status_codes::NotFound);
        }
    } else {
        req.reply(web::http::status_codes::NotFound);
    }
}

void gdalcubes_server::handle_post(web::http::http_request req) {
    std::vector<std::string> path = web::uri::split_path(web::uri::decode(req.relative_uri().path()));
    std::map<std::string, std::string> query_pars = web::uri::split_query(web::uri::decode(req.relative_uri().query()));
    //    std::for_each(path.begin(), path.end(), [](std::string s) { std::cout << s << std::endl; });
    //    std::for_each(query_pars.begin(), query_pars.end(), [](std::pair<std::string, std::string> s) { std::cout << s.first << ":" << s.second << std::endl; });

    if (!path.empty()) {
        if (path[0] == "file") {
            std::cout << "POST /file" << std::endl;
            std::string fname;
            if (query_pars.find("name") != query_pars.end()) {
                fname = query_pars["name"];
            } else {
                fname = utils::generate_unique_filename();
            }
            fname = (_workdir / fname).string();
            if (boost::filesystem::exists(fname)) {
                if (req.headers().has("Content-Length")) {
                    if (req.headers().content_length() == boost::filesystem::file_size(fname)) {
                        req.reply(web::http::status_codes::OK);
                    } else {
                        req.reply(web::http::status_codes::Conflict);
                    }
                } else {
                    req.reply(web::http::status_codes::Conflict);
                }
            } else {
                boost::filesystem::create_directories(boost::filesystem::path(fname).parent_path());
                auto fstream = std::make_shared<concurrency::streams::ostream>();
                pplx::task<void> t = concurrency::streams::fstream::open_ostream(fname).then(
                    [fstream, &req](concurrency::streams::ostream outFile) {
                        *fstream = outFile;
                        concurrency::streams::istream indata = req.body();
                        uint32_t maxbytes = 1024 * 1024 * 8;  // Read up to 8MiB
                        while (!indata.is_eof()) {
                            indata.read(fstream->streambuf(), maxbytes).wait();
                        }
                        fstream->close().wait();
                    });
                t.wait();
                req.reply(web::http::status_codes::OK, fname.c_str(), "text/plain");
            }
        } else if (path[0] == "cube") {
            if (path.size() == 1) {
                std::cout << "POST /cube" << std::endl;
                // we do not use cpprest JSON library here
                uint32_t id;
                req.extract_string(true).then([&id, this](std::string s) {
                                            std::shared_ptr<cube> c = cube_factory::create_from_json(nlohmann::json::parse(s));
                                            id = get_unique_id();
                                            _mutex_cubestore.lock();
                                            _cubestore.insert(std::make_pair(id, c));
                                            _mutex_cubestore.unlock();
                                        })
                    .wait();
                req.reply(web::http::status_codes::OK, std::to_string(id), "text/plain");
            } else if (path.size() == 4) {
                uint32_t cube_id = std::stoi(path[1]);
                uint32_t chunk_id = std::stoi(path[2]);

                std::string cmd = path[3];
                if (cmd == "start") {
                    std::cout << "POST /cube/" << cube_id << "/" << chunk_id << "/start" << std::endl;

                    if (_cubestore.find(cube_id) == _cubestore.end()) {
                        req.reply(web::http::status_codes::NotFound, "ERROR in /POST /cube/{cube_id}/{chunk_id}/start: cube is not available", "text/plain");
                    } else if (chunk_id >= _cubestore[cube_id]->count_chunks()) {
                        req.reply(web::http::status_codes::NotFound, "ERROR in /POST /cube/{cube_id}/{chunk_id}/start: invalid chunk_id given", "text/plain");
                    }

                    // if already in queue, executing, or finished, do not compute again
                    else if (server_chunk_cache::instance()->has(std::make_pair(cube_id, chunk_id)) ||
                             _chunk_read_requests_set.find(std::make_pair(cube_id, chunk_id)) != _chunk_read_requests_set.end() ||
                             _chunk_read_executing.find(std::make_pair(cube_id, chunk_id)) != _chunk_read_executing.end()) {
                        req.reply(web::http::status_codes::OK);
                    } else {
                        _mutex_worker_threads.lock();
                        if (_worker_threads.size() < config::instance()->get_server_worker_threads_max()) {
                            // immediately start a thread

                            _worker_threads.push_back(std::thread([this, cube_id, chunk_id]() {
                                _mutex_chunk_read_requests.lock();
                                _chunk_read_requests.push_back(std::make_pair(cube_id, chunk_id));
                                _chunk_read_requests_set.insert(std::make_pair(cube_id, chunk_id));
                                _mutex_chunk_read_requests.unlock();
                                while (1) {
                                    _mutex_chunk_read_requests.lock();
                                    while (!_chunk_read_requests.empty()) {
                                        _mutex_chunk_read_executing.lock();
                                        uint32_t xcube_id = _chunk_read_requests.front().first;
                                        uint32_t xchunk_id = _chunk_read_requests.front().second;
                                        _chunk_read_executing.insert(_chunk_read_requests.front());
                                        _chunk_read_requests_set.erase(_chunk_read_requests.front());
                                        _chunk_read_requests.pop_front();
                                        _mutex_chunk_read_requests.unlock();
                                        _mutex_chunk_read_executing.unlock();

                                        _mutex_cubestore.lock();
                                        std::shared_ptr<cube> c = _cubestore[xcube_id];
                                        _mutex_cubestore.unlock();

                                        std::shared_ptr<chunk_data> dat = c->read_chunk(xchunk_id);

                                        server_chunk_cache::instance()->add(std::make_pair(xcube_id, xchunk_id), dat);

                                        _mutex_chunk_read_executing.lock();
                                        _chunk_read_executing.erase(std::make_pair(xcube_id, xchunk_id));
                                        _mutex_chunk_read_executing.unlock();

                                        // notify waiting download requests
                                        if (_chunk_cond.find(std::make_pair(xcube_id, xchunk_id)) != _chunk_cond.end()) {
                                            _chunk_cond[std::make_pair(xcube_id, xchunk_id)].first.notify_all();
                                        }

                                        _mutex_chunk_read_requests.lock();
                                    }
                                    _mutex_chunk_read_requests.unlock();

                                    std::unique_lock<std::mutex> lock(_mutex_worker_cond);
                                    _worker_cond.wait(lock);
                                }
                            }));

                            _mutex_worker_threads.unlock();
                            req.reply(web::http::status_codes::OK);
                        }

                        else {
                            _mutex_worker_threads.unlock();
                            std::lock_guard<std::mutex> lock(_mutex_worker_cond);
                            // add to queue
                            _mutex_chunk_read_requests.lock();
                            _chunk_read_requests.push_back(std::make_pair(cube_id, chunk_id));
                            _mutex_chunk_read_requests.unlock();
                            _worker_cond.notify_all();
                            req.reply(web::http::status_codes::OK);
                        }
                    }
                } else {
                    req.reply(web::http::status_codes::NotFound);
                }
            } else {
                req.reply(web::http::status_codes::NotFound);
            }

        } else {
            req.reply(web::http::status_codes::NotFound);
        }
    } else {
        req.reply(web::http::status_codes::NotFound);
    }
}

void gdalcubes_server::handle_put(web::http::http_request req) {
    req.reply(web::http::status_codes::MethodNotAllowed);
}

void gdalcubes_server::handle_head(web::http::http_request req) {
    std::vector<std::string> path = web::uri::split_path(web::uri::decode(req.relative_uri().path()));
    std::map<std::string, std::string> query_pars = web::uri::split_query(web::uri::decode(req.relative_uri().query()));
    if (!path.empty() && path[0] == "file") {
        std::cout << "HEAD /file" << std::endl;
        std::string fname;
        if (query_pars.find("name") != query_pars.end()) {
            fname = query_pars["name"];
            fname = (_workdir / fname).string();
            if (boost::filesystem::exists(fname)) {
                if (query_pars.find("size") != query_pars.end()) {
                    if (std::stoi(query_pars["size"]) == boost::filesystem::file_size(fname)) {
                        req.reply(web::http::status_codes::OK);  // File exists and has the same size
                    } else {
                        req.reply(web::http::status_codes::Conflict);  // File exists but has different size
                    }
                } else {
                    req.reply(web::http::status_codes::BadRequest);  // File exists but Content Length missing for comparison
                }
            } else {
                req.reply(web::http::status_codes::NoContent);  // File does not exist yet
            }

        } else {
            req.reply(web::http::status_codes::BadRequest);  // no name given in request
        }
    } else {
        req.reply(web::http::status_codes::NotFound);  //
    }
}

void print_usage() {
    std::cout << "Usage: gdalcubes_server [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Run gdalcubes_server and wait for incoming HTTP requests."
              << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -b, --basepath              Base path for all API endpoints, defaults to /gdalcubes/api/" << std::endl;
    std::cout << "  -p, --port                  The port where gdalcubes_server is listening, defaults to 1111" << std::endl;
    std::cout << "  -t, --worker_threads        Maximum number of threads perfoming chunk reads, defaults to 1" << std::endl;
    std::cout << "  -D, --dir                   Working directory where files are stored, defaults to {TEMPDIR}/gdalcubes" << std::endl;
    std::cout << "      --ssl                   Use HTTPS (currently not implemented)" << std::endl;
    std::cout << std::endl;
}

std::unique_ptr<gdalcubes_server> server;
int main(int argc, char* argv[]) {
    config::instance()->gdalcubes_init();

    namespace po = boost::program_options;
    // see https://stackoverflow.com/questions/15541498/how-to-implement-subcommands-using-boost-program-options

    po::options_description global_args("Options");
    global_args.add_options()("help,h", "")("version", "")("basepath,b", po::value<std::string>()->default_value("/gdalcubes/api"), "")("port,p", po::value<uint16_t>()->default_value(1111), "")("ssl", "")("worker_threads,t", po::value<uint16_t>()->default_value(1), "")("dir,D", po::value<std::string>()->default_value((boost::filesystem::temp_directory_path() / "gdalcubes").string()), "");

    po::variables_map vm;

    bool ssl;
    try {
        po::parsed_options parsed = po::command_line_parser(argc, argv).options(global_args).allow_unregistered().run();
        po::store(parsed, vm);
        if (vm.count("version")) {
            std::cout << "gdalcubes_server " << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH
                      << " built on " << __DATE__ << " " << __TIME__;
            std::cout << " linked against " << GDALVersionInfo("--version")
                      << std::endl;  // TODO add version info for other linked libraries
            return 0;
        }
        if (vm.count("help")) {
            print_usage();
            return 0;
        }

        ssl = vm.count("ssl") > 0;
    } catch (...) {
        std::cout << "ERROR in gdalcubes_server: cannot parse arguments." << std::endl;
        return 1;
    }

    std::unique_ptr<gdalcubes_server> srv = std::unique_ptr<gdalcubes_server>(
        new gdalcubes_server("0.0.0.0", vm["port"].as<uint16_t>(), vm["basepath"].as<std::string>(), ssl,
                             vm["dir"].as<std::string>()));

    config::instance()->set_server_worker_threads_max(vm["worker_threads"].as<uint16_t>());

    srv->open().wait();
    std::cout << "gdalcubes_server waiting for incoming HTTP requests on " << srv->get_service_url() << "." << std::endl;
    std::cout << "Press ENTER to exit" << std::endl;
    std::string line;
    std::getline(std::cin, line);

    srv->close().wait();
    config::instance()->gdalcubes_cleanup();
    return 0;
}