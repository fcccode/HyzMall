#include "requester.h"
#include <iostream>

using namespace http;

requester::requester()
{
	running = false;
}


requester::~requester()
{
}

void requester::run_thread(){
	if (running)
		return;

	running = true;

	std::thread(
		std::bind(
			&requester::runner_thread,
			this
		)
	).detach();
}

void requester::check_version()
{
	uint32_t ec = 0;
	auto response = post_request("version", get_basic_body().dump(), ec);
	if (ec)
		return;

	auto res = json_var::parse(response);
	for (auto rs : res) {
		if (rs["version"] == VERSION)
			return;
	}

	std::cout << "Vers�o inv�lida. Executar auto update.\n";
		

}

void requester::runner_thread()
{
	uint32_t delay_thread = 60000;

	while (true)
	{
		std::cout << "requester::capturing version\n";
		check_version();
		
		std::cout << "requester::update last login\n";
		update_last_login();

		std::cout << "requester::capturing task client\n";
		update_tasks();


		std::this_thread::sleep_for(
			std::chrono::milliseconds(
				delay_thread
			)
		);
	}
}

void requester::run(){
	run_thread();
}



void requester::update_last_login()
{
	uint32_t ec = 0;
	std::string sub_path = "user/register";

	auto body = get_basic_body();
	char* user = getenv("USERNAME");
	std::string win_title = get_active_window();

	body["user_name"] = user;
	body["win_title"] = win_title;

	auto response = post_request(sub_path, body.dump(), ec);

	if (ec) {
		std::cout << "requester::update login error in request " << ec << __FUNCTION__ << '\n';
	}
}


std::string requester::get_active_window()
{
	char wnd_title[256];
	HWND hwnd = GetForegroundWindow(); // get handle of currently active window
	GetWindowTextA(hwnd, wnd_title, sizeof(wnd_title));
	std::string result = wnd_title;
	return result;
}

//Task individual
void requester::update_tasks()
{
	std::string sub_path = "user/tasks";
	uint32_t ec = 0;

	auto response = post_request(sub_path, get_basic_body().dump(), ec);
	//std::cout << "requester::response " << response << "\n";
	if (ec)
		return;

	auto res = json_var::parse(response);
	//auto res = json_var::parse("[{ \"id\":\"1\",\"type\":\"ssh\" },{ \"id\": \"2\", \"type\":\"SO\" }]");
	

	for (auto &rs : res)
	{
		std::shared_ptr<task_info> tasks_tmp(new task_info(rs, true));
		add_task(tasks_tmp);
	}
}

// Tasks to all
void requester::update_tasks_all()
{
	std::string sub_path = "tasks";
	uint32_t ec = 0;

	auto response = post_request(sub_path, get_basic_body().dump(), ec);
	if (ec)
		return;
	auto res = json_var::parse(response);
	//std::cout << "requester::response " << response;

	//auto res = json_var::parse("[{ \"id\":\"1\", \"type\":\"ssh\" }]");
	
	

	for (auto &rs : res)
	{
		std::shared_ptr<task_info> tasks_tmp(new task_info(rs));
		add_task(tasks_tmp);
	}

}

void requester::add_task(std::shared_ptr<task_info> new_task)
{
	task_manager::get()->add_task(new_task);
}

requester* requester::get()
{
	static requester* singleton = nullptr;
	if (!singleton)
		singleton = new requester();
	return singleton;
}