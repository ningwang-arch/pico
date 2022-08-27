#include "pico/pico.h"


struct Student
{
    int id;
    std::string name;
    int age;
    std::string address;
    std::string phone;

    REFLECTION(student, id, name, age, address, phone);

    friend std::ostream& operator<<(std::ostream& os, const Student& s) {
        os << "Student(" << s.id << ", " << s.name << ", " << s.age << ", " << s.address << ", "
           << s.phone << ")";
        return os;
    }

    bool operator<(const Student& other) const { return id < other.id; }
};

struct Data
{
    int integer;
    std::string string;
    std::vector<Student> students;
    std::map<std::string, Student> students_map;
    std::set<Student> students_set;

    REFLECTION(Data, integer, string, students, students_map, students_set);

    friend std::ostream& operator<<(std::ostream& os, const Data& d) {
        os << "Data(" << d.integer << ", " << d.string << ", [";
        for (auto& i : d.students) {
            os << i << ", ";
        }
        os << "], {";
        for (auto& i : d.students_map) {
            os << i.first << ": " << i.second << ", ";
        }
        os << "}, {";
        for (auto& i : d.students_set) {
            os << i << ", ";
        }
        os << "})";
        return os;
    }
};

void test() {
    pico::RedisConnection conn;
    conn.connect("127.0.0.1", 6379);

    auto ret = conn.set("key", "value");
    std::cout << "set: " << ret << std::endl;

    auto ret2 = conn.get<std::string>("key");
    if (ret2.status == pico::RedisStatus::REDIS_STATUS_OK) {
        std::cout << "get: " << ret2.data << std::endl;
    }
}

void test_01() {
    Student s;
    s.id = 1;
    s.name = "小红";
    s.age = 18;
    s.address = "beijing";
    s.phone = "123456789";

    pico::RedisConnection conn;
    conn.connect("127.0.0.1", 6379);

    auto ret = conn.set("student", s);
    std::cout << "set: " << ret << std::endl;

    auto ret2 = conn.get<Student>("student");
    if (ret2.status == pico::RedisStatus::REDIS_STATUS_OK) {
        std::cout << "get: " << ret2.data << std::endl;
    }
}

void test_02() {
    Student s;
    s.id = 1;
    s.name = "小红";
    s.age = 18;
    s.address = "beijing";
    s.phone = "123456789";

    Data d;
    d.integer = 1;
    d.string = "hello";
    d.students.push_back(s);
    d.students_map[s.name] = s;
    d.students_set.insert(s);

    auto conn = pico::RedisManager::getInstance()->getConnection();

    auto ret = conn->set("data", d);
    std::cout << "set: " << ret << std::endl;

    auto ret2 = conn->get<Data>("data");
    if (ret2.status == pico::RedisStatus::REDIS_STATUS_OK) {
        std::cout << "get: " << ret2.data << std::endl;
    }
}

void test_03() {
    auto conn = pico::RedisManager::getInstance()->getConnection();

    auto ret = conn->hset("hash", "key", "value");
    std::cout << "hset: " << ret << std::endl;
    auto ret2 = conn->hget<std::string>("hash", "key");
    if (ret2.status == pico::RedisStatus::REDIS_STATUS_OK) {
        std::cout << "hget: " << ret2.data << std::endl;
    }

    ret = conn->hdel("hash", "key");
    std::cout << "hdel: " << ret << std::endl;
}


int main(int argc, char* argv[]) {
    pico::EnvManager::getInstance()->init(argc, argv);
    pico::Config::LoadFromConfDir(pico::EnvManager::getInstance()->getConfigPath());
    // test();
    // test_01();
    test_02();
    // test_03();
    return 0;
}
