//
// Created by sly on 6/6/23.
//

#ifndef SLY_CONFIG_H
#define SLY_CONFIG_H

#include<memory>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "log.h"
#include "util.h"
#include <yaml-cpp/yaml.h>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <iostream>

namespace sylar{
    ///配置变量基类
    class ConfigVarBase{
    public:
        //智能指针
        typedef std::shared_ptr<ConfigVarBase> ptr;
        //构造函数初始化
        //description = ""意味着如果不懈第二个参数将默认空字符，增加灵活性
        ConfigVarBase(const std::string& name, const std::string& description = ""):
        m_name(name),
        m_description(description)
        {
            std::transform(m_name.begin(), m_name.end(), m_name.begin(),::tolower);
        }
        //虚栖沟函数，方便多态
        virtual ~ConfigVarBase() {}

        //返回配置参数名称
        const std::string& getName() const { return m_name;}

        //@brief 返回配置参数的描述
        const std::string& getDescription() const { return m_description;}

        //转换成字符串函数
        //纯虚函数，派生类必须实现该函数，否则报错
        virtual std::string toString() = 0;

        //字符串初始化
        virtual bool fromString(const std::string& val) = 0;

        //
        virtual std::string getTypeName() const = 0;

    protected:
        // 配置参数的名称
        std::string m_name;
        // 配置参数的描述
        std::string m_description;
    };

    /**
    * @brief 类型转换模板类(F 源类型, T 目标类型)
    */
    template<class F, class T>
    class LexicalCast {
    public:
        /**
         * @brief 类型转换
         * @param[in] v 源类型值
         * @return 返回v转换后的目标类型
         * @exception 当类型不可转换时抛出异常
         */
        T operator()(const F& v) {
            return boost::lexical_cast<T>(v);
        }
    };

    /**
    * @brief 类型转换模板类片特化(YAML String 转换成 std::vector<T>)
    */
    template<class T>
    class LexicalCast<std::string, std::vector<T> > {
    public:
        //()它接受一个 std::string 类型的参数 v，并返回一个 std::vector<T> 类型的结果
        std::vector<T> operator()(const std::string& v) {
            //YAML::Load(v) 将输入的 YAML 字符串解析为 YAML::Node 对象
            YAML::Node node = YAML::Load(v);
            typename std::vector<T> vec;
            std::stringstream ss;
            for(size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                vec.push_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

    /**
    * @brief 类型转换模板类片特化(std::vector<T> 转换成 YAML String)
    */
    template<class T>
    class LexicalCast<std::vector<T>, std::string> {
    public:
        std::string operator()(const std::vector<T>& v) {
            YAML::Node node;
            for(auto& i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    /**
    * @brief 类型转换模板类片特化(YAML String 转换成 std::list<T>)
    */
    template<class T>
    class LexicalCast<std::string, std::list<T> > {
    public:
        //()它接受一个 std::string 类型的参数 v，并返回一个 std::vector<T> 类型的结果
        std::list<T> operator()(const std::string& v) {
            //YAML::Load(v) 将输入的 YAML 字符串解析为 YAML::Node 对象
            YAML::Node node = YAML::Load(v);
            typename std::list<T> vec;
            std::stringstream ss;
            for(size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                vec.push_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

    /**
* @brief 类型转换模板类片特化(std::list<T> 转换成 YAML String)
*/
    template<class T>
    class LexicalCast<std::list<T>, std::string> {
    public:
        std::string operator()(const std::list<T>& v) {
            YAML::Node node;
            for(auto& i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    /**
    * @brief 类型转换模板类片特化(YAML String 转换成 std::set<T>)
    */
    template<class T>
    class LexicalCast<std::string, std::set<T> > {
    public:
        //()它接受一个 std::string 类型的参数 v，并返回一个 std::vector<T> 类型的结果
        std::set<T> operator()(const std::string& v) {
            //YAML::Load(v) 将输入的 YAML 字符串解析为 YAML::Node 对象
            YAML::Node node = YAML::Load(v);
            typename std::set<T> vec;
            std::stringstream ss;
            for(size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                vec.insert(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

    /**
* @brief 类型转换模板类片特化(std::set<T> 转换成 YAML String)
*/
    template<class T>
    class LexicalCast<std::set<T>, std::string> {
    public:
        std::string operator()(const std::set<T>& v) {
            YAML::Node node;
            for(auto& i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    /**
* @brief 类型转换模板类片特化(YAML String 转换成 std::unordered_set<T>)
*/
    template<class T>
    class LexicalCast<std::string, std::unordered_set<T> > {
    public:
        //()它接受一个 std::string 类型的参数 v，并返回一个 std::vector<T> 类型的结果
        std::unordered_set<T> operator()(const std::string& v) {
            //YAML::Load(v) 将输入的 YAML 字符串解析为 YAML::Node 对象
            YAML::Node node = YAML::Load(v);
            typename std::unordered_set<T> vec;
            std::stringstream ss;
            for(size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                vec.insert(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

    /**
* @brief 类型转换模板类片特化(std::unordered_set<T> 转换成 YAML String)
*/
    template<class T>
    class LexicalCast<std::unordered_set<T>, std::string> {
    public:
        std::string operator()(const std::unordered_set<T>& v) {
            YAML::Node node;
            for(auto& i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::map<std::string, T>)
 */
    template<class T>
    class LexicalCast<std::string, std::map<std::string, T> > {
    public:
        std::map<std::string, T> operator()(const std::string& v) {
            YAML::Node node = YAML::Load(v);
            typename std::map<std::string, T> vec;
            std::stringstream ss;
            for(auto it = node.begin();
                it != node.end(); ++it) {
                ss.str("");
                ss << it->second;
                vec.insert(std::make_pair(it->first.Scalar(),
                                          LexicalCast<std::string, T>()(ss.str())));
            }
            return vec;
        }
    };

/**
 * @brief 类型转换模板类片特化(std::map<std::string, T> 转换成 YAML String)
 */
    template<class T>
    class LexicalCast<std::map<std::string, T>, std::string> {
    public:
        std::string operator()(const std::map<std::string, T>& v) {
            YAML::Node node(YAML::NodeType::Map);
            for(auto& i : v) {
                node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    /**
    * @brief 类型转换模板类片特化(YAML String 转换成 std::unordered_map<std::string, T>)
    */
    template<class T>
    class LexicalCast<std::string, std::unordered_map<std::string, T> > {
    public:
        std::unordered_map<std::string, T> operator()(const std::string& v) {
            YAML::Node node = YAML::Load(v);
            typename std::unordered_map<std::string, T> vec;
            std::stringstream ss;
            for(auto it = node.begin();
                it != node.end(); ++it) {
                ss.str("");
                ss << it->second;
                vec.insert(std::make_pair(it->first.Scalar(),
                                          LexicalCast<std::string, T>()(ss.str())));
            }
            return vec;
        }
    };

    /**
    * @brief 类型转换模板类片特化(std::unordered_map<std::string, T> 转换成 YAML String)
    */
    template<class T>
    class LexicalCast<std::unordered_map<std::string, T>, std::string> {
    public:
        std::string operator()(const std::unordered_map<std::string, T>& v) {
            YAML::Node node(YAML::NodeType::Map);
            for(auto& i : v) {
                node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };



    ///配置参数模板子类,保存对应类型的参数值
    template<class T, class FromStr = LexicalCast<std::string, T>,
            class ToStr = LexicalCast<T, std::string> >
    class ConfigVar : public ConfigVarBase{
    public:
        typedef std::shared_ptr<ConfigVar> ptr;

        ///回调函数
        typedef std::function<void (const T& old_value, const  T& new_value)> on_change_cb;

        ///构造函数
        /**
         *
         * @param name 参数名称
         * @param default_value 参数默认值
         * @param description  参数描述
         */
        ConfigVar(const std::string& name, const T& default_value,
                  const std::string& description = ""):
                ConfigVarBase(name, description),
                m_val(default_value){

        }
        /**
        * @brief 将参数值转换成YAML String
        * @exception 当转换失败抛出异常
        */
        std::string toString() override {
            try {
                //将T类别的m_val转化为字符串函数
                //return boost::lexical_cast<std::string>(m_val);
                //ToStr std::string operator()(const T&)
                return ToStr()(m_val);
            }
            //std::exception& e 表示返回一个异常的饮用，通过e访问异常对象
            catch (std::exception& e) {
                //使用自定义的宏流式输出
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::toString exception "
                                                  << e.what() << " convert: " << typeid(m_val).name() << " to string";
            }
            //如果发生异常或者转换失败，会直接诶返回一个空字符串
            return "";
        }

        /**
        * @brief 从YAML String 转成参数的值
        * @exception 当转换失败抛出异常
        */
        bool fromString(const std::string& val) override {
            try {
                //m_val = boost::lexical_cast<T>(val);
                //FromStr 把string转换成复杂类型 FromStr T operator()(const std::string&)
                setValue(FromStr()(val));
            } catch (std::exception& e) {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::fromString exception "
                                                  << e.what() << " convert: string to " << typeid(m_val).name()
                                                  << " - " << val;
            }
            return false;
        }

        const T getValue() const {return m_val;}

        void setValue(const T& v) {
            if(v == m_val){
                return;
            }
            for(auto& i : m_cbs){
                i.second(m_val, v);
            }
            m_val = v;
        }

        //返回函数类型名字
        std::string getTypeName() const override {return typeid(T).name();}
        /**
            * @brief 添加变化回调函数
            * @return 返回该回调函数对应的唯一id,用于删除回调
            */
        uint64_t addListener(on_change_cb cb){
            static uint64_t s_fun_id = 0;
            ++s_fun_id;
            m_cbs[s_fun_id] = cb;
            return s_fun_id;
        }

        //删除函数，删除key数值
        void delListener(uint64_t key){
            m_cbs.erase(key);
        }
        // 回调返回函数 找到对应对象
        on_change_cb getListener(uint64_t key){
            auto it = m_cbs.find(key);
            return it == m_cbs.end() ? nullptr : it->second;
        }

        void clearListener(){
            m_cbs.clear();
        }

    private:
        T m_val;
        // 回调变更函数组，uint_64t key 要求唯一值，如果不是的话，容易被修改可以用hash
        std::map<uint64_t, on_change_cb> m_cbs;

    };

    /**
    * @brief ConfigVar的管理类
    * @details 提供便捷的方法创建/访问ConfigVar
    */
    class Config{
    public:
        typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

        template<class T>
                static typename  ConfigVar<T>::ptr    Lookup(const std::string& name,
                                                          const T& default_value,
                                                          const std:: string& description = ""){
                    //如果找到名字返回，没找到返回空指针
                    auto it = GetDatas().find(name);
                    if(it!=GetDatas().end())
                    {
                        auto tmp = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
                        if(tmp){
                            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Lookup name= " << name << " exists";
                            return tmp;
                        } else{
                            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name = [" << name << "] exists but type not! ["
                                                            << typeid(T).name() << "] real_type= "
                                                            << it->second->getTypeName()
                                                            << it->second->toString();
                            return nullptr;
                        }
                    }

                    if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789")!=
                    std::string::npos){
                        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name invaild " << name;
                        throw std::invalid_argument(name);
                    }

                    typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
                    GetDatas()[name] = v;
                    return v;
                }

        template<class T>
                static typename ConfigVar<T>::ptr Lookup(const std::string& name){
                    auto it = GetDatas().find(name);
                    if(it == GetDatas().end()){

                        return nullptr;
                    }
                    return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
                }
        /**
        * @brief 使用YAML::Node初始化配置模块
        */
        static void LoadFromYaml(const YAML::Node& root);

        static ConfigVarBase::ptr LookupBase(const std::string& name);

    private:
        // 为什么这样定义P17 30.01min
        static ConfigVarMap& GetDatas(){
            static ConfigVarMap s_datas;
            return s_datas;
        }


    };
}



#endif //SLY_CONFIG_H
