astyle -v --formatted --options=astyle-options.ini --recursive "../include/*.h,*.hpp"
astyle -v --formatted --options=astyle-options.ini --recursive "../src/*.cpp"
astyle -v --formatted --options=astyle-options.ini --recursive "../test/*.cpp"

pause