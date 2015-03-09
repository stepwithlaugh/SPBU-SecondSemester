#include <iostream>
#include <string>
#include <set>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include "tinyxml/tinyxml.h"
#include "tinyxml/tinyxmlerror.cpp"
#include "tinyxml/tinyxmlparser.cpp"

namespace fs = boost::filesystem;
std::string test;

//Данные будем записывать в структуру, а структуру копировать в вектор
struct Fileinfo {
	std::string path;
	std::string hash;
	int size;
	char flag;
};

//Записываем в xml файл, туториал лежит на http://www.grinninglizard.com/tinyxmldocs/tutorial0.html
//Всё записываю через атрибуты, можно и по-другому - но не суть.
void save2xml(std::string filename, std::vector<Fileinfo> vec_finfo) {
	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration("1.0", "", "");
	doc.LinkEndChild(decl);
	for (Fileinfo it : vec_finfo) {
		TiXmlElement * element = new TiXmlElement("File");
		doc.LinkEndChild(element);
		element->SetAttribute("path", it.path.c_str());
		element->SetAttribute("size", it.size);
		element->SetAttribute("hash", it.hash.c_str());
		element->SetAttribute("flag", it.flag);
		TiXmlText * text = new TiXmlText("");
		element->LinkEndChild(text);
	}
	doc.SaveFile(filename);
}

void get_dir_list(fs::directory_iterator iterator, std::vector<Fileinfo> * vec_finfo) {  //выводит список файлов и папок в директории
	Fileinfo finfo; //объявление структуры, в которую будем записывать данные и складывать их в вектор
	for (; iterator != fs::directory_iterator(); ++iterator)
	{
		if (fs::is_directory(iterator->status())) { //если наткнулись на папку, то рекурсивно запускаем эту же функцию для этой папки
			fs::directory_iterator sub_dir(iterator->path());
			get_dir_list(sub_dir, vec_finfo);

		}
		else //а если нет, то записываем в структуру имя, размер, хеш, и флажок (понадобится чуть позже, когда будем искать изменения в файлах)
		{
			finfo.path = iterator->path().string();
			finfo.size = fs::file_size(iterator->path());
			finfo.hash = "NOT YET";
			finfo.flag = 'R';
			vec_finfo->push_back(finfo);
		}

	}
}

int main() {
	std::string path, dirpath;
	//std::cout << "Folder path:" << std::endl;
	std::getline(std::cin, path); 
	// Кстати, путь можно скопировать и вставлять через меню, кликая по иконке запускаемого приложения в левом верхнем углу.
	// Клик левой кнопкой по иконке - изменить - вставить
	std::cout << std::endl;
	//replace_slashes(& path);
	std::vector<Fileinfo> vec_finfo; //Вектор в который мы будет складывать объекты нашей структуры
	fs::directory_iterator home_dir(path);
	get_dir_list(home_dir, &vec_finfo);

	//Выводим список файлов, размеров и т.д.:
	for (Fileinfo element : vec_finfo) {
		std::cout << element.path << std::endl <<
			element.size << std::endl <<
			element.hash << std::endl <<
			element.flag << std::endl;
	}

	save2xml("example.xml", vec_finfo);  //сохраняем полученное в хмл файле с именем example.xml, создастся он в папке где находится main.cpp
	std::cin.clear();
	fflush(stdin);
	std::cin.get();
	return 0;
}

/*
http://sourceforge.net/projects/boost/files/boost/1.57.0/ - тут скачать (zip файл)

http://stackoverflow.com/questions/2629421/how-to-use-boost-in-visual-studio-2010 (cм. второй ответ)

выполнить в директории с бустом:
b2 toolset=msvc-12.0 --build-type=complete --libdir=C:\Boost\lib\i386 install

Скопировать все файлы 
из 
C:\Boost\lib\i386
в
C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\lib



XML:
http://www.boost.org/doc/libs/1_42_0/libs/property_tree/examples/debug_settings.cpp
http://www.boost.org/doc/libs/1_42_0/doc/html/boost_propertytree/tutorial.html

XML:
скачать tinyxml отсюда: 
http://sourceforge.net/projects/tinyxml/?source=typ_redirect
разархивировать папку tinyxml в папку где лежит твой main.cpp

найти в папке sln файл, запустить в visual studio, и запустить его.
Затем скопировать все .obj и .lib файлы
из
Путь к твоему проекту\tinyxml\Debugtinyxml
в
C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\lib

В твоем основном проекте зайти в Properties 
Потом - Linker -> Input -> Additional Dependencies
добавить туда строчку "tinyxml.lib" (без кавычек)
Tutorial по tinyxml: http://www.grinninglizard.com/tinyxmldocs/tutorial0.html
*/
