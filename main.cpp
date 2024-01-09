#include <httplib.h>
#include <iostream>
#include <cstdio>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>
#include <crow.h>
#include <crow/middlewares/cookie_parser.h>

using json = nlohmann::json;

int main()
{
    crow::SimpleApp app;

    std::string logo;
    logo = " ____  __  __ ____       _    __  __    _     ________  _   _ \n"
           "/ ___||  \\/  / ___|     / \\  |  \\/  |  / \\   |__  / _ \\| \\ | |\n"
           "\\___ \\| |\\/| \\___ \\    / _ \\ | |\\/| | / _ \\    / / | | |  \\| |\n"
           " ___) | |  | |___) |  / ___ \\| |  | |/ ___ \\  / /| |_| | |\\  |\n"
           "|____/|_|  |_|____/  /_/   \\_\\_|  |_/_/   \\_\\/____\\___/|_| \\_|\n"
           "                                                              \n";
    printf("%s",logo.c_str());
//设置前端显示界面
    CROW_ROUTE(app, "/")
            ([](crow::response &res) {
                std::ifstream file("frontend/index.html", std::ifstream::in | std::ifstream::binary);
                std::stringstream ss;
                ss << file.rdbuf();
                res.write(ss.str());
                res.add_header("Content-Type", "text/html");
                res.end();
                return;
            });
    CROW_ROUTE(app, "/<string>")
            ([](const crow::request &req, crow::response &res, std::string filename) {
                std::ifstream file("frontend/" + filename, std::ifstream::in | std::ifstream::binary);
                if (!file) {
                    res.code = 404;
                    res.end();
                    return;
                }
                std::stringstream ss;
                ss << file.rdbuf();
                res.write(ss.str());
                res.set_header("Content-Type", "text/html");
                res.end();
                return;
            });
    //生成手机号码
    CROW_ROUTE(app, "/api/v1/generate")
            ([&](const crow::request &req, crow::response &res) {
                std::string figure = req.url_params.get("figure");
                std::string number = req.url_params.get("number");
                long long int phone = std::stoll(figure);
                std::ofstream write("data.txt");
                if (!write.is_open())
                {
                   res.write("Error opening file");
                   res.end();
                }
                for (int i = 0; i < std::stoll(number); i++) {
                    write << std::to_string(phone) << std::endl;
                    phone = phone + 1;
                }
                write.close();
                res.write("Success");
                res.end();
            });
    //将手机号码发送到服务器
    CROW_ROUTE(app, "/api/v1/send")
            ([&](const crow::request &req, crow::response &res) {
                //获取生成的手机号
                std::string line;
                std::vector<std::string> lines;
                std::ifstream read("data.txt");
                if (!read.is_open()) {
                    res.write("You have not generate number file");
                    res.end();
                }
                while(std::getline(read,line)){
                    lines.push_back(line);
                }
                int phone_number = lines.size();
                //获取服务器地址
                std::ifstream server("server.txt");
                if (!server.is_open()) {
                    res.write("You have not set server");
                    res.end();
                }
                std::string server_ip;
                std::vector<std::string> server_ips;
                while(std::getline(server,server_ip)){
                    server_ips.push_back(server_ip);
                }
                int server_number = server_ips.size();
                server.close();

                //分配发送手机号码

                for (int i = 0; i < server_number; i++) {
                    std::string data_name = "data_part" + std::to_string(i) + ".txt";
                    std::ofstream output_data(data_name.c_str());
                    if (!output_data.is_open()) {
                        res.write("Error opening file");
                        res.end();
                    }
                    for (int j = 0; j < phone_number / server_number; j++) {
                        output_data << lines[i * (phone_number / server_number) + j] << "\n";
                    }
                    output_data.close();
                    httplib::Client cli(server_ips[i].c_str());
                    auto res = cli.Post("/api/v1/CheckAccount", data_name.c_str(), "text/plain");
                    auto res2 = cli.Get("/api/v1/Run");
                }
            });
    //查看状态
    CROW_ROUTE(app, "/api/v1/status")
            ([&](const crow::request &req, crow::response &res) {
                std::ifstream server("server.txt");
                if (!server.is_open()) {
                    res.write("You have not set server");
                    res.end();
                }
                std::string server_ip;
                std::vector<std::string> server_ips;
                while(std::getline(server,server_ip)){
                    server_ips.push_back(server_ip);
                }
                int server_number = server_ips.size();
                server.close();
                double total_success = 0;
                double total_failed = 0;
                double total_all = 0;
                for (int i = 0; i < server_number; i++) {
                    httplib::Client cli(server_ips[i].c_str());
                    auto result = cli.Get("/api/v1/GetStatus");
                    std::string status = result->body;
                    json j = json::parse(status);
                    double success = j["success"];
                    double failed = j["failed"];
                    double all = j["all"];
                    total_success = total_success + success;
                    total_failed = total_failed + failed;
                    total_all = total_all + all;
                }
                double percent = (total_success + total_failed) / total_all * 100;
                json req_data;
                req_data["success"] = total_success;
                req_data["failed"] = total_failed;
                req_data["percent"] = percent;
                res.write(req_data.dump());
                res.end();
            });
    //强制暂停
    CROW_ROUTE(app, "/api/v1/stop")
            ([&](const crow::request &req, crow::response &res) {
                std::ifstream server("server.txt");
                if (!server.is_open()) {
                    res.write("You have not set server");
                    res.end();
                }
                std::string server_ip;
                std::vector<std::string> server_ips;
                while(std::getline(server,server_ip)){
                    server_ips.push_back(server_ip);
                }
                int server_number = server_ips.size();
                server.close();
                for (int i = 0; i < server_number; i++) {
                    httplib::Client cli(server_ips[i].c_str());
                    auto result = cli.Get("/api/v1/ForceStop");
                }
                res.write("Success");
                res.end();
            });
    //清空
    CROW_ROUTE(app, "/api/v1/clear")
            ([&](const crow::request &req, crow::response &res) {
                std::ifstream server("server.txt");
                if (!server.is_open()) {
                    res.write("You have not set server");
                    res.end();
                }
                std::string server_ip;
                std::vector<std::string> server_ips;
                while(std::getline(server,server_ip)){
                    server_ips.push_back(server_ip);
                }
                int server_number = server_ips.size();
                server.close();
                for (int i = 0; i < server_number; i++) {
                    httplib::Client cli(server_ips[i].c_str());
                    auto result = cli.Get("/api/v1/ClearAll");
                }
                res.write("Success");
                res.end();
            });
    //获取结果
    CROW_ROUTE(app, "/api/v1/result")
            ([&](const crow::request &req, crow::response &res) {
                std::ifstream server("server.txt");
                if (!server.is_open()) {
                    res.write("You have not set server");
                    res.end();
                }
                std::string server_ip;
                std::vector<std::string> server_ips;
                while(std::getline(server,server_ip)){
                    server_ips.push_back(server_ip);
                }
                int server_number = server_ips.size();
                server.close();
                std::string status;
                for (int i = 0; i < server_number; i++) {
                    httplib::Client cli(server_ips[i].c_str());
                    auto result = cli.Get("/api/v1/GetResult?type=success");
                    status += result->body + "\n";
                }
                res.write(status);
                res.end();
            });
    app.bindaddr("0.0.0.0")
    .port(5588)
    .multithreaded()
    .run();

    return 0;
}
