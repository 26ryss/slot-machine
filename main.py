"""
Hand Landmark Model
8: INDEX_FINGER_TIP
12: MIDDLE_FINGER_TIP
"""

import cv2
import mediapipe as mp

def calc_distance(p1, p2):
    return ((p1.x - p2.x) ** 2 + (p1.y - p2.y) ** 2 + (p1.z - p2.z) ** 2) ** 0.5

CLOCK = 3
DISTANCE = 0.05

mp_drawing = mp.solutions.drawing_utils
mp_drawing_styles = mp.solutions.drawing_styles
mp_hands = mp.solutions.hands

count = 0
prev_state = None

cap = cv2.VideoCapture(0)
with mp_hands.Hands(
    model_complexity=0,
    min_detection_confidence=0.5,
    min_tracking_confidence=0.5) as hands:
    
    while cap.isOpened():
        action = []
        success, image = cap.read()
        if not success:
            print("Ignoring empty camera frame.")
            continue
        image.flags.writeable = False
        image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        results = hands.process(image)
        # 検出された手の骨格をカメラ画像に重ねて描画
        image.flags.writeable = True
        image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)
        
        # 一つ目の手だけを検出対象とする
        if results.multi_hand_landmarks:
            hand_landmarks = results.multi_hand_landmarks[0]
            mp_drawing.draw_landmarks(
                image,
                hand_landmarks,
                mp_hands.HAND_CONNECTIONS,
                mp_drawing_styles.get_default_hand_landmarks_style(),
                mp_drawing_styles.get_default_hand_connections_style()
            )
        
        if count == CLOCK:
            count = -1
            if results.multi_hand_landmarks:
                if prev_state:
                    cur_index_finger_tip = hand_landmarks.landmark[8]
                    cur_middle_finger_tip = hand_landmarks.landmark[12]
                    prev_index_finger_tip = prev_state.landmark[8]
                    prev_middle_finger_tip = prev_state.landmark[12]
                
                    if prev_index_finger_tip.z - cur_index_finger_tip.z > DISTANCE:
                        action.append(0)
                    if cur_middle_finger_tip.y - prev_middle_finger_tip.y > DISTANCE:
                        action.append(1)
                prev_state = hand_landmarks

        count += 1
        with open('action.txt', 'w') as f:
            f.write(str(action))
        cv2.imshow('MediaPipe Hands', cv2.flip(image, 1))

        if cv2.waitKey(5) & 0xFF == 27:
            break
cap.release()