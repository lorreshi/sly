//
// Created by sly on 6/6/23.
//
#include "config.h"
#include <list>
#include <map>

namespace sylar{

    //就是 ConfigVarMap s_data; 静态变量的类外声明，加上定义域Config：：
    Config::ConfigVarMap Config::s_datas;

    //查找背后的基类指针，找到了返回对应的指针
    ConfigVarBase::ptr Config::LookupBase(const std::string& name){
       auto it = s_datas.find(name);
        return  it == s_datas.end() ? nullptr : it->second;
    }



//"A.B", 10
//A:
//  B: 10
//  C: str
    // 辅助函数，用于递归遍历YAML配置节点并生成一个节点列表
    static void ListAllMember(const std::string& prefix,
                              const YAML::Node& node,
                              std::list<std::pair<std::string, const YAML::Node> >& output) {
        // 检查前缀是否合法
        if(prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678")
           != std::string::npos) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
            return;
        }
        // 将当前节点加入输出列表
        output.push_back(std::make_pair(prefix, node));
        // 如果当前节点是映射类型，继续遍历其子节点
        if(node.IsMap()) {
            for(auto it = node.begin();
                it != node.end(); ++it) {
                // 递归调用ListAllMember函数处理子节点
                ListAllMember(prefix.empty() ? it->first.Scalar()
                                             : prefix + "." + it->first.Scalar(), it->second, output);
            }
        }
    }

    // 从YAML配置中加载配置数据
    void Config::LoadFromYaml(const YAML::Node& root) {
        // 创建一个节点列表
        std::list<std::pair<std::string, const YAML::Node> > all_nodes;
        // 使用辅助函数ListAllMember递归遍历YAML配置并生成节点列表
        ListAllMember("", root, all_nodes);
        // 遍历节点列表，根据配置项的名称查找对应的配置变量并更新其值
        for(auto& i : all_nodes) {
            std::string key = i.first;
            if(key.empty()) {
                continue;
            }
            // 将配置项的名称转换为小写
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            // 查找配置变量
            ConfigVarBase::ptr var = LookupBase(key);
            // 如果找到对应的配置变量，根据节点的类型更新配置变量的值
            if(var) {
                if(i.second.IsScalar()) {
                    var->fromString(i.second.Scalar());
                } else {
                    std::stringstream ss;
                    ss << i.second;
                    var->fromString(ss.str());
                }
            }
        }
    }
}