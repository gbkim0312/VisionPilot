#pragma once
#include <string>

namespace vp::domain::model
{

// YOLOv8 (COCO Dataset) 80 Classes
enum class ClassId : int
{
    PERSON = 0,
    BICYCLE = 1,
    CAR = 2,
    MOTORCYCLE = 3,
    AIRPLANE = 4,
    BUS = 5,
    TRAIN = 6,
    TRUCK = 7,
    BOAT = 8,
    TRAFFIC_LIGHT = 9,
    FIRE_HYDRANT = 10,
    STOP_SIGN = 11,
    PARKING_METER = 12,
    BENCH = 13,
    BIRD = 14,
    CAT = 15,
    DOG = 16,
    HORSE = 17,
    SHEEP = 18,
    COW = 19,
    ELEPHANT = 20,
    BEAR = 21,
    ZEBRA = 22,
    GIRAFFE = 23,
    BACKPACK = 24,
    UMBRELLA = 25,
    HANDBAG = 26,
    TIE = 27,
    SUITCASE = 28,
    FRISBEE = 29,
    SKIS = 30,
    SNOWBOARD = 31,
    SPORTS_BALL = 32,
    KITE = 33,
    BASEBALL_BAT = 34,
    BASEBALL_GLOVE = 35,
    SKATEBOARD = 36,
    SURFBOARD = 37,
    TENNIS_RACKET = 38,
    BOTTLE = 39,
    WINE_GLASS = 40,
    CUP = 41,
    FORK = 42,
    KNIFE = 43,
    SPOON = 44,
    BOWL = 45,
    BANANA = 46,
    APPLE = 47,
    SANDWICH = 48,
    ORANGE = 49,
    BROCCOLI = 50,
    CARROT = 51,
    HOT_DOG = 52,
    PIZZA = 53,
    DONUT = 54,
    CAKE = 55,
    CHAIR = 56,
    COUCH = 57,
    POTTED_PLANT = 58,
    BED = 59,
    DINING_TABLE = 60,
    TOILET = 61,
    TV = 62,
    LAPTOP = 63,
    MOUSE = 64,
    REMOTE = 65,
    KEYBOARD = 66,
    CELL_PHONE = 67,
    MICROWAVE = 68,
    OVEN = 69,
    TOASTER = 70,
    SINK = 71,
    REFRIGERATOR = 72,
    BOOK = 73,
    CLOCK = 74,
    VASE = 75,
    SCISSORS = 76,
    TEDDY_BEAR = 77,
    HAIR_DRIER = 78,
    TOOTHBRUSH = 79,
    UNKNOWN = -1
};

// 유틸리티 클래스 (변환용)
class ClassIdHelper
{
public:
    // int -> Enum 변환
    static ClassId fromInt(int id)
    {
        if (id >= 0 && id <= 79)
        {
            return static_cast<ClassId>(id);
        }
        return ClassId::UNKNOWN;
    }

    // Enum -> String 변환 (디버깅/로깅용)
    static std::string toString(ClassId id)
    {
        switch (id)
        {
        case ClassId::PERSON:
            return "Person";
        case ClassId::BICYCLE:
            return "Bicycle";
        case ClassId::CAR:
            return "Car";
        case ClassId::MOTORCYCLE:
            return "Motorcycle";
        case ClassId::AIRPLANE:
            return "Airplane";
        case ClassId::BUS:
            return "Bus";
        case ClassId::TRAIN:
            return "Train";
        case ClassId::TRUCK:
            return "Truck";
        case ClassId::BOAT:
            return "Boat";
        case ClassId::TRAFFIC_LIGHT:
            return "Traffic Light";
        case ClassId::FIRE_HYDRANT:
            return "Fire Hydrant";
        case ClassId::STOP_SIGN:
            return "Stop Sign";
        case ClassId::PARKING_METER:
            return "Parking Meter";
        case ClassId::BENCH:
            return "Bench";
        case ClassId::BIRD:
            return "Bird";
        case ClassId::CAT:
            return "Cat";
        case ClassId::DOG:
            return "Dog";
        case ClassId::HORSE:
            return "Horse";
        case ClassId::SHEEP:
            return "Sheep";
        case ClassId::COW:
            return "Cow";
        case ClassId::ELEPHANT:
            return "Elephant";
        case ClassId::BEAR:
            return "Bear";
        case ClassId::ZEBRA:
            return "Zebra";
        case ClassId::GIRAFFE:
            return "Giraffe";
        case ClassId::BACKPACK:
            return "Backpack";
        case ClassId::UMBRELLA:
            return "Umbrella";
        case ClassId::HANDBAG:
            return "Handbag";
        case ClassId::TIE:
            return "Tie";
        case ClassId::SUITCASE:
            return "Suitcase";
        case ClassId::FRISBEE:
            return "Frisbee";
        case ClassId::SKIS:
            return "Skis";
        case ClassId::SNOWBOARD:
            return "Snowboard";
        case ClassId::SPORTS_BALL:
            return "Sports Ball";
        case ClassId::KITE:
            return "Kite";
        case ClassId::BASEBALL_BAT:
            return "Baseball Bat";
        case ClassId::BASEBALL_GLOVE:
            return "Baseball Glove";
        case ClassId::SKATEBOARD:
            return "Skateboard";
        case ClassId::SURFBOARD:
            return "Surfboard";
        case ClassId::TENNIS_RACKET:
            return "Tennis Racket";
        case ClassId::BOTTLE:
            return "Bottle";
        case ClassId::WINE_GLASS:
            return "Wine Glass";
        case ClassId::CUP:
            return "Cup";
        case ClassId::FORK:
            return "Fork";
        case ClassId::KNIFE:
            return "Knife";
        case ClassId::SPOON:
            return "Spoon";
        case ClassId::BOWL:
            return "Bowl";
        case ClassId::BANANA:
            return "Banana";
        case ClassId::APPLE:
            return "Apple";
        case ClassId::SANDWICH:
            return "Sandwich";
        case ClassId::ORANGE:
            return "Orange";
        case ClassId::BROCCOLI:
            return "Broccoli";
        case ClassId::CARROT:
            return "Carrot";
        case ClassId::HOT_DOG:
            return "Hot Dog";
        case ClassId::PIZZA:
            return "Pizza";
        case ClassId::DONUT:
            return "Donut";
        case ClassId::CAKE:
            return "Cake";
        case ClassId::CHAIR:
            return "Chair";
        case ClassId::COUCH:
            return "Couch";
        case ClassId::POTTED_PLANT:
            return "Potted Plant";
        case ClassId::BED:
            return "Bed";
        case ClassId::DINING_TABLE:
            return "Dining Table";
        case ClassId::TOILET:
            return "Toilet";
        case ClassId::TV:
            return "TV";
        case ClassId::LAPTOP:
            return "Laptop";
        case ClassId::MOUSE:
            return "Mouse";
        case ClassId::REMOTE:
            return "Remote";
        case ClassId::KEYBOARD:
            return "Keyboard";
        case ClassId::CELL_PHONE:
            return "Cell Phone";
        case ClassId::MICROWAVE:
            return "Microwave";
        case ClassId::OVEN:
            return "Oven";
        case ClassId::TOASTER:
            return "Toaster";
        case ClassId::SINK:
            return "Sink";
        case ClassId::REFRIGERATOR:
            return "Refrigerator";
        case ClassId::BOOK:
            return "Book";
        case ClassId::CLOCK:
            return "Clock";
        case ClassId::VASE:
            return "Vase";
        case ClassId::SCISSORS:
            return "Scissors";
        case ClassId::TEDDY_BEAR:
            return "Teddy Bear";
        case ClassId::HAIR_DRIER:
            return "Hair Drier";
        case ClassId::TOOTHBRUSH:
            return "Toothbrush";
        default:
            return "Unknown";
        }
    }
};

struct BoundingBox
{
    float x; // Center X or Top-Left X (구현에 따라 정의, 보통 Left-Top)
    float y; // Center Y or Top-Left Y
    float width;
    float height;
};

struct Detection
{
    ClassId class_id;  // YOLO Class ID (0: person, etc.)
    float confidence;  // 신뢰도 (0.0 ~ 1.0)
    BoundingBox bbox;  // 위치 정보
    std::string label; // (Optional) "person", "car" 등 디버깅용
};

} // namespace vp::domain::model