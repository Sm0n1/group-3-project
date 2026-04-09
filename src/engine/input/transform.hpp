namespace clayborne::input::transform {
    enum class tag {
        button_press,
        button_press_and_release,
        // other types of transformations
    };

    union data {
        // some types of transformations such as axis to button requires deadzones and thresholds
    };

    struct descriptor {
        tag t;
        data d;
    };
}