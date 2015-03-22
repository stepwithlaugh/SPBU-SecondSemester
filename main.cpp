//Dan Krupsky, 108

/*
Всё компилим через MinGW
http://sourceforge.net/projects/boost/files/boost/1.57.0/ - тут скачать (zip файл)
!!! Как установить BOOST:
записать в PATH путь к MinGW\bin
Зайти в директории буста tools\build\v2
выполнить
bootstrap mingw
tools\build\v2\b2 toolset=gcc --build-type=complete stage --with-filesystem
Библиотеки будут в директория буста  \ stage\lib
Переименовать соответствующие .a файлы (нам нужны filesystem и system) соответсвенно в libboost_filesystem.a и libboost_system.a
!!! Как установить протобаф:
распаковать протобаф в home директории msys'a в mingw, зайти в эту директорию
./configure --prefix=`cd C:/MinGW; pwd -W`
make
make install

зайти в папку с исходниками, теперь proto файл можно компилировать командой protoc:
protoc example.proto
На выходу получается два файла: example.pb.h и example.pb.cc, в main.cpp делаем инклуд на .h файл

!!! Команда для компиляции (из MSYS):
g++ main.cpp pbfile.pb.cc -std=c++11 -IC:/MinGW/msys/1.0/home/boost_1_57_0/ -LC:/MinGW/msys/1.0/home/boost_1_57_0/stage/lib -lboost_filesystem -lboost_system -lprotobuf

Для работы не из msys'a надо найти следующие dll в директррии mingw и сложить их в папку с exe файлом:
libgcc_s_dw2-1.dll
libprotobuf-9.dll
libstdc++-6.dll
*/

#include <iostream>
#include <fstream>  //Чтение и запись
#include <string>
#include <boost/filesystem.hpp>  //BOOST::FILESYSTEM с помощью которого считываем директорию
#include "pbfile.pb.h" //файл, который сделал protoc
#include "md6/md6.h"

namespace fs = boost::filesystem;
std::string test;

//Данные будем записывать в структуру, а структуру копировать в вектор
struct Fileinfo {
	std::string path;
	std::string hash;
	int size;
	std::string flag;
};

//Записываем через протобаф. Filelist - внешняя структура, в которую кладем элементы Filep
void savepbuf(std::string filename, std::vector<Fileinfo> vec_finfo) {
	nsofdir::ArrFilep flist;
	nsofdir::Filep * file_entry;
	std::ofstream output(filename, std::ofstream::binary);
	for (Fileinfo it : vec_finfo) {
		//Запись, просто по сделанным методам протобафа
		file_entry = flist.add_filep();
		file_entry->set_filepath(it.path);
		file_entry->set_size(it.size);
		file_entry->set_mdsixhash(it.hash); // добавить позже

	}
	//Вывод файла
	flist.PrintDebugString();
	//Записываем в output файл
	flist.SerializeToOstream(&output);
	output.close();
}

//Загрузка записанного файла (записывает в файл, но пока не засовывает в вектор - доделать позже
void loadpbuf(std::string filename, std::vector<Fileinfo> vec_finfo) {
	nsofdir::ArrFilep flist;  //Filelist, в который считаем файл
	nsofdir::Filep * file_entry;
	std::ifstream input("filelist.pb", std::ofstream::binary); // Открываем наш записанный файл
	flist.ParseFromIstream(&input);  //Парсим из файла
	//flist.PrintDebugString(); // Вывод файла
	input.close();
}

//выводит список файлов и папок в директории
void get_dir_list(fs::directory_iterator iterator, std::vector<Fileinfo> & vec_finfo, Fileinfo & finfo, std::ifstream & ifs) {
	for (; iterator != fs::directory_iterator(); ++iterator)
	{
		if (fs::is_directory(iterator->status())) {
			//если наткнулись на папку, то рекурсивно запускаем эту же функцию для этой папки
			fs::directory_iterator sub_dir(iterator->path());
			get_dir_list(sub_dir, vec_finfo, finfo, ifs);

		}
		else
			//а если нет, то записываем в структуру имя, размер, хеш, и флажок (понадобится чуть позже, когда будем искать изменения в файлах)
		{

			finfo.path = iterator->path().string();
			//исправляем косяк с \ перед файлом в пути, так работает filesystem :-(
			std::replace(finfo.path.begin(), finfo.path.end(), '\\', '/');
			finfo.size = fs::file_size(iterator->path());
			ifs.open(finfo.path, std::ios_base::binary);
			std::string strifs((std::istreambuf_iterator<char>(ifs)),
				(std::istreambuf_iterator<char>()));
			finfo.hash = md6(strifs);
			ifs.close();
			finfo.flag = 'R';
			vec_finfo.push_back(finfo);
		}

	}
}

int main() {
	std::ofstream myfile;
	std::string path, dirpath;
	//объявление структуры, в которую будем записывать данные и складывать их в вектор
	Fileinfo finfo;
	//Инпут, через который мы считаем хеш
	std::ifstream ifs;
	std::cout << "Folder path:" << std::endl;
	std::getline(std::cin, path);
	// Кстати, путь можно скопировать и вставлять через меню, кликая по иконке запускаемого приложения в левом верхнем углу.
	// Клик левой кнопкой по иконке - изменить - вставить.
	//Вектор, в который мы будет складывать объекты нашей структуры
	std::vector<Fileinfo> vec_finfo;
	//Вектор, в который мы будем считывать из файла, чтобы сравнить две дерева файлов
	std::vector<Fileinfo> vec_finfo_old;
	//Итератор, который будет бегать по папкам и файлам
	fs::directory_iterator home_dir(path);
	//Запуск функции, которая запишем в вектор все файлы
	get_dir_list(home_dir, vec_finfo, finfo, ifs);

	//Выводим список файлов, размеров и т.д. из вектора. Просто для дебага.
	/*
	for (Fileinfo element : vec_finfo) {
	std::cout << element.path << std::endl <<
	element.size << std::endl <<
	element.hash << std::endl <<
	element.flag << std::endl;
	}
	*/
	//сохраняем полученное в хмл файле с именем example.xml, создастся он в папке где находится main.cpp
	savepbuf("filelist.pb", vec_finfo);
	//loadpbuf("example.xml", vec_finfo_old);  //загрузка файла
	std::cin.clear();
	fflush(stdin);
	std::cin.get();
	return 0;
}