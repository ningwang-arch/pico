CREATE database IF NOT EXISTS `mapper`;

use mapper;

DROP TABLE IF EXISTS `t_school`;

CREATE TABLE `t_school` (
    `id` int NOT NULL AUTO_INCREMENT,
    `name` varchar(255) DEFAULT NULL,
    `create_time` datetime DEFAULT NULL,
    PRIMARY KEY (`id`)
) ENGINE = InnoDB AUTO_INCREMENT DEFAULT CHARSET = utf8;

-- 班级表

CREATE TABLE `class` (
    `class_id` int NOT NULL AUTO_INCREMENT,
    `class_name` varchar(255) DEFAULT NULL,
    `school_id` int DEFAULT NULL,
    PRIMARY KEY (`class_id`)
) ENGINE = InnoDB AUTO_INCREMENT DEFAULT CHARSET = utf8;

-- 学生表

CREATE TABLE `student` (
    `id` int NOT NULL AUTO_INCREMENT,
    `name` varchar(255) DEFAULT NULL,
    `class_id` int DEFAULT NULL,
    `create_time` datetime DEFAULT NULL,
    PRIMARY KEY (`id`)
) ENGINE = InnoDB DEFAULT CHARSET = utf8;