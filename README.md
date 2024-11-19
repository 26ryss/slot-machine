# slot-machine

## c++のコンパイル
```
g++ -std=c++11 main.cpp -o main \
-framework OpenAL -framework OpenGL -framework GLUT \
`pkg-config --cflags --libs opencv4` -lalure \
-I/opt/homebrew/Cellar/alure/1.2_1/include/OpenAL \
-L/opt/homebrew/Cellar/alure/1.2_1/lib -w
```

## pythonの環境構築
```
python -m venv .env

source .env/bin/activate

pip install -r requirements.txt
```