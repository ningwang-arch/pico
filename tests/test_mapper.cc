#include <chrono>


#include "pico/env.h"
#include "pico/iomanager.h"
#include "pico/mapper/common/mapper.hpp"
#include "pico/mapper/entity/entity_wrapper.h"


using std::chrono::system_clock;
struct School
{
    int id = 0;
    std::string name;
    std::time_t createTime = {};

    School() = default;

    School(const std::string& name, time_t createTime)
        : name(name)
        , createTime(createTime) {}

    School(int id, const std::string& name, time_t createTime)
        : id(id)
        , name(name)
        , createTime(createTime) {}

    friend std::ostream& operator<<(std::ostream& os, const School& school) {
        os << "id: " << school.id << " name: " << school.name
           << " createTime: " << school.createTime;
        return os;
    }
};

ResultMap(EntityMap(::School, "t_school"), PropertyMap(id, ColumnType::Id), PropertyMap(name),
          PropertyMap(createTime));

struct Class
{
    int classId = 0;
    std::string className;
    int schoolId = 0;

    friend std::ostream& operator<<(std::ostream& os, const Class& aClass) {
        os << "classId: " << aClass.classId << " className: " << aClass.className
           << " schoolId: " << aClass.schoolId;
        return os;
    }
};

ResultMap(EntityMap(Class), PropertyMap(classId, ColumnType::Id), PropertyMap(className),
          PropertyMap(schoolId));

struct Student
{
    int id = 0;
    std::string name;
    Class clazz;
    std::time_t createTime;

    friend std::ostream& operator<<(std::ostream& os, const Student& student) {
        os << "id: " << student.id << " name: " << student.name << " clazz: " << student.clazz
           << " createTime: " << student.createTime;
        return os;
    }
};

ResultMap(EntityMap(Student), PropertyMap(id, ColumnType::Id), PropertyMap(name),
          PropertyMap(clazz, "class_id", JoinType::OneToOne, &Class::classId),
          PropertyMap(createTime));

struct SchoolForMany
{
    int id = 0;
    std::string name;
    std::time_t createTime = {};
    std::vector<Class> clazzs;

    friend std::ostream& operator<<(std::ostream& os, const SchoolForMany& school) {
        os << "id: " << school.id << " name: " << school.name
           << " createTime: " << school.createTime;
        os << " clazzs: [";
        for (int i = 0; i < (int)school.clazzs.size(); i++) {
            os << " { ";
            os << school.clazzs[i];
            os << "} ";
        }
        os << "]";
        return os;
    }
};

ResultMap(EntityMap(SchoolForMany, "t_school"), PropertyMap(id, ColumnType::Id), PropertyMap(name),
          PropertyMap(createTime),
          //特别注意,“id”为School表的连接字段id,&Class::schoolId为class表的连接字段
          PropertyMap(clazzs, "id", JoinType::OneToMany, &Class::schoolId));


void test_insert() {
    pico::Mapper<School> sc_mapper;
    sc_mapper.use("sql_1");
    std::cout << sc_mapper.insert(School("My_school", system_clock::to_time_t(system_clock::now())))
              << std::endl;
    std::cout << sc_mapper.insert(
                     School("World-School", system_clock::to_time_t(system_clock::now())))
              << std::endl;
    std::cout << sc_mapper.insert(School("My-School", system_clock::to_time_t(system_clock::now())))
              << std::endl;
}


void test_select() {
    pico::Mapper<School> sc_mapper;
    sc_mapper.use("sql_1");
    auto results = sc_mapper.selectAll();
    for (auto school : results) { std::cout << school << std::endl; }
}

void test_update() {
    pico::Mapper<School> sc_mapper;
    sc_mapper.use("sql_1");
    std::cout << sc_mapper.updateByPrimaryKey(
                     School(7, "Hello-School_update", system_clock::to_time_t(system_clock::now())))
              << std::endl;
}

void test_delete() {
    pico::Mapper<School> sc_mapper;
    sc_mapper.use("sql_1");
    std::cout << sc_mapper.deleteByPrimaryKey(7) << std::endl;
}

void test_select_with_cond() {
    pico::Mapper<School> sc_mapper;
    sc_mapper.use("sql_1");
    pico::Base<School> base;
    auto criteria = base.createCriteria();
    criteria->andLike(&School::name, "%My%");
    auto results = sc_mapper.select(base);
    for (auto& result : results) { std::cout << result << std::endl; }
}

void test_one2one() {
    pico::Mapper<Student> stu_mapper;
    pico::Base<Student> base;
    auto criteria = base.createCriteria();
    criteria->andGreaterThan(&Student::id, 1);
    criteria->andEqualTo(&Class::className, "classA");
    auto results = stu_mapper.select(base);
    for (auto& result : results) { std::cout << result << std::endl; }
}

void test_one2many() {
    pico::Mapper<SchoolForMany> sc_mapper;
    auto results = sc_mapper.selectAll();
    for (auto& ret : results) { std::cout << ret << std::endl; }
}

void test_iommanager() {
    pico::IOManager iom(2);
    iom.schedule(&test_select);
}

int main(int argc, char* argv[]) {
    pico::EnvManager::getInstance()->init(argc, argv);
    pico::Config::LoadFromConfDir(pico::EnvManager::getInstance()->getConfigPath());
    // test_insert();
    // test_select();
    // test_update();
    // test_delete();
    // test_select_with_cond();
    // test_one2one();
    // test_one2many();
    test_iommanager();
    return 0;
}
