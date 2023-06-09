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

    protected:
        // 配置参数的名称
        std::string m_name;
        // 配置参数的描述
        std::string m_description;
    };

    ///配置参数模板子类,保存对应类型的参数值
    template<class T>
    class ConfigVar : public ConfigVarBase{
    public:
        typedef std::shared_ptr<ConfigVar> ptr;
        ///构造函数
        /**
         *
         * @param name 参数名称
         * @param default_value 参数默认值
         * @param description  参数描述
         */
        ConfigVar(const std::string& name, const T& default_value,
                  const std::string& description = " "):
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
                return boost::lexical_cast<std::string>(m_val);
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
                m_val = boost::lexical_cast<T>(val);
            } catch (std::exception& e) {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::fromString exception "
                                                  << e.what() << " convert: string to " << typeid(m_val).name();
            }
            return false;
        }

        const T getValue() const {return m_val;}
        void setValue(const T& v) {m_val = v;}
    private:
        T m_val;

    };

    /**
    * @brief ConfigVar的管理类
    * @details 提供便捷的方法创建/访问ConfigVar
    */
    class Config{
    public:
        typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

        template<class T>
                static typename  ConfigVar<T>::ptr Lookup(const std::string& name,
                                                          const T& default_value,
                                                          const std:: string& description = " "){
                    auto tmp = Lookup<T>(name);
                    if(tmp){
                        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Lookup name= " << name << "exists";
                        return tmp;
                    }
                    if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789")!=
                    std::string::npos){
                        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name invaild " << name;
                        throw std::invalid_argument(name);
                    }

                    typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
                    s_datas[name] = v;
                    return v;
                }

        template<class T>
                static typename ConfigVar<T>::ptr Lookup(const std::string& name){
                    auto it = s_datas.find(name);
                    if(it == s_datas.end()){
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
        static ConfigVarMap s_datas;
    };
}



#endif //SLY_CONFIG_H
