//
// Created by sly on 6/6/23.
//
#include "../sly/config.h"
#include "../sly/log.h"
#include <yaml-cpp/yaml.h>
#include <iostream>

////自定义解析的配置 就是默认配置
sylar::ConfigVar<int>::ptr g_int_valuex_config =
        sylar::Config::Lookup("system.port", (int)8080, "system port");
sylar::ConfigVar<float>::ptr g_int_value_config =
        sylar::Config::Lookup("system.port", (float)8080, "system port");
sylar::ConfigVar<float>::ptr g_float_value_config =
        sylar::Config::Lookup("system.value", (float)10.2f, "system value");

sylar::ConfigVar<std::vector<int> >::ptr g_int_vec_value_config =
        sylar::Config::Lookup("system.int_vec", std::vector<int>{1,3}, "system int_vec");
sylar::ConfigVar<std::list<int> >::ptr g_int_list_value_config =
        sylar::Config::Lookup("system.int_list", std::list<int>{1,2}, "system int_list");
sylar::ConfigVar<std::set<int> >::ptr g_int_set_value_config =
        sylar::Config::Lookup("system.int_set", std::set<int>{1,2}, "system int_set");
sylar::ConfigVar<std::unordered_set<int> >::ptr g_int_uset_value_config =
        sylar::Config::Lookup("system.int_uset", std::unordered_set<int>{1,2}, "system int_uset");
sylar::ConfigVar<std::map<std::string, int> >::ptr g_str_int_map_value_config =
        sylar::Config::Lookup("system.str_int_map", std::map<std::string, int>{{"k",2}}, "system str_int_map");
sylar::ConfigVar<std::unordered_map<std::string, int> >::ptr g_str_int_umap_value_config =
        sylar::Config::Lookup("system.str_int_umap", std::unordered_map<std::string, int>{{"k",2}}, "system str_int_map");

void print_yaml(const YAML::Node& node, int level) {
    if(node.IsScalar()) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << node.Scalar() << " - " << node.Tag() << " - " << level;
    } else if(node.IsNull()) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "NULL - " << node.Tag() << " - " << level;
    } else if(node.IsMap()) {
        for(auto it = node.begin();
            it != node.end(); ++it) {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << it->first << " - " << it->second.Tag() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    } else if(node.IsSequence()) {
        for(size_t i = 0; i < node.size(); ++i) {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << i << " - " << node[i].Tag() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}


void test_yaml(){
    YAML::Node root = YAML::LoadFile("/home/sly/CLionProjects/sly/cmake-build-debug/bin/log.yml");
    //print_yaml(root, 0);
    //SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << root;
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << root["test"].IsDefined();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << root["logs"].IsDefined();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << root;
}

void test_config() {
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before: " << g_float_value_config->toString();

#define XX(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue(); \
        for(auto& i : v) { \
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name ": " << i; \
        }                       \
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }
#define XX_M(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue(); \
        for(auto& i : v) { \
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name ": {" \
                    << i.first << " - " << i.second << "}"; \
        } \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }

    XX(g_int_vec_value_config, int_vec, before);
    XX(g_int_list_value_config, int_list, before);
    XX(g_int_set_value_config, int_set, before);
    XX(g_int_uset_value_config, int_uset, before);
    XX_M(g_str_int_map_value_config, int_uset, before);
    XX_M(g_str_int_umap_value_config, int_uset, before);

    YAML::Node root = YAML::LoadFile("/home/sly/CLionProjects/sly/cmake-build-debug/bin/test.yml");
    sylar::Config::LoadFromYaml(root);

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: " << g_float_value_config->toString();
    XX(g_int_vec_value_config, int_vec, after);
    XX(g_int_list_value_config, int_list, after);
    XX(g_int_set_value_config, int_set, after);
    XX(g_int_uset_value_config, int_uset, after);
    XX_M(g_str_int_map_value_config, int_uset, after);
    XX_M(g_str_int_umap_value_config, int_uset, after);

}
//定义一个类
class Person
{
public:
    Person() {};
    std::string m_name;
    int m_age = 0;
    bool m_sex = 0;

    std::string toString() const {
        std::stringstream ss;
        ss << "[Person name= " << m_name
           << " age= " << m_age
           << " sex= " << m_sex
           << "]";
        return ss.str();
    }
    bool operator==(const Person& oth) const {
        return m_name == oth.m_name && m_age == oth.m_age && m_sex == oth.m_age;
    }

};

//给Person的类的lookup做一个片特化  (重点看看片特化知识)
namespace sylar {

    template<>
    class LexicalCast<std::string, Person> {
    public:
        Person operator()(const std::string& v) {
            YAML::Node node = YAML::Load(v);
            Person p;
            p.m_name = node["name"].as<std::string>();
            p.m_age = node["age"].as<int>();
            p.m_sex = node["sex"].as<bool>();
            return p;
        }
    };

    template<>
    class LexicalCast<Person, std::string> {
    public:
        std::string operator()(const Person& p) {
            YAML::Node node;
            node["name"] = p.m_name;
            node["age"] = p.m_age;
            node["sex"] = p.m_sex;
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

}

//自定义
sylar::ConfigVar<Person>::ptr g_person =
        sylar::Config::Lookup("class.person", Person(), "system person");
//自定义复杂类型map嵌套person -》 可以解析各种复杂的数据结构
sylar::ConfigVar<std::map<std::string, Person> >::ptr g_person_map =
        sylar::Config::Lookup("class.map", std::map<std::string, Person>(), "system person");

//处理自定义类，比如来一个自定义struct，怎么让yaml识别并且处理
void test_class(){
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) <<"before: " << g_person->getValue().toString() << " - " << g_person->toString();

#define XX_PM(g_var, prefix) \
    { \
        auto m = g_person_map->getValue(); \
        for(auto& i : m) { \
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) <<  prefix << ": " << i.first << " - " << i.second.toString(); \
        } \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) <<  prefix << ": size=" << m.size(); \
    }

    g_person->addListener([](const Person& old_value, const Person& new_value){
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << " old value= " << old_value.toString()
                                         << " new value= " << new_value.toString();
    });

    XX_PM(g_person_map, "class.map before");

    YAML::Node root = YAML::LoadFile("/home/sly/CLionProjects/sly/cmake-build-debug/bin/test.yml");
    sylar::Config::LoadFromYaml(root);

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) <<"after: " << g_person->getValue().toString() << " - " << g_person->toString();
    XX_PM(g_person_map, "class.map after");
}

void test_log(){
    static sylar::Logger::ptr system_log = SYLAR_LOG_NAME("system");
    SYLAR_LOG_INFO(system_log) << "hello system" << std::endl;
    std::cout << sylar::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile("/home/sly/CLionProjects/sly/cmake-build-debug/bin/log.yml");
    sylar::Config::LoadFromYaml(root);
    std::cout << "==================" << std:: endl;
    std::cout << sylar::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    std::cout << "==================" << std:: endl;
    std::cout << root << std::endl;
    SYLAR_LOG_INFO(system_log) << "hello system" << std::endl;

    system_log->setFormatter("%d - %m%n");
    SYLAR_LOG_INFO(system_log) << "hello system" << std::endl;
}

void test_visit(){
    sylar::Config::Visit([](sylar::ConfigVarBase::ptr var){
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "name=" << var->getName()
                                 << "description=" << var->getDescription()
                                 << "typename=" << var->getTypeName()
                                 << "value=" << var->toString();
    });
}

int main(int argc, char** argv){
    //SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_int_value_config->getValue();
    //SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_float_value_config->toString();

    //test_yaml();
    //test_config();
    //test_class();
    //test_log();
    test_visit();
    return 0;
}