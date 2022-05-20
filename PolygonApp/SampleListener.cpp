
#include <windows.h>
#include "Leap.h"
#include "Header.h"
#include "SampleListener.h"
#include "initializePolygon.h"

using namespace Leap;
/***********LeapMotionの処理***************/

void SampleListener::onInit(const Controller& controller) {
    std::cout << "Initialized" << std::endl;
}

void SampleListener::onConnect(const Controller& controller) {
    std::cout << "Connected" << std::endl;
    /* 使用するジェスチャーをセットする */
    controller.enableGesture(Gesture::TYPE_CIRCLE);
    controller.enableGesture(Gesture::TYPE_KEY_TAP);
    controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
    controller.enableGesture(Gesture::TYPE_SWIPE);
}

void SampleListener::onDisconnect(const Controller& controller) {
    std::cout << "Disconnected" << std::endl;
}

void SampleListener::onExit(const Controller& controller) {
    std::cout << "Exited" << std::endl;
}

void SampleListener::onFrame(const Controller& controller) {
    Frame frame = controller.frame();// 最新のFrameを取得する
    HandList handList = frame.hands();// 全ての手の情報を取得する
    FingerList allFingerList = frame.fingers();// 全ての指の情報を取得する
    GestureList gesturesInFrame = frame.gestures();// 全てのジェスチャーの情報を取得する

    //それぞれの個数を表示する
    // printf("\n\n\nhands:%d, fingers:%2d, gestures:%d\n",
    //   handList.count(), allFingerList.count(), gesturesInFrame.count());

    int i, j;

    /********************************************
    * 頂点を更新する前に保存 （更新中は保存しない）
    *******************************************/
    if (movingFlag == -1 && rotatingFlag == -1 && scalingFlag == -1) {
        prePointer = pointer;
        for (int i = 0; i < latitudeNUM + 1; i++) {
            for (int j = 0; j < longitudeNUM; j++) {
                prePoint[i][j] = point[i][j];
            }
        }
    }

    for (i = 0; i < handList.count(); i++) {    // 片手づつ処理を行う
        Hand hand = handList[i];
        /* 変形 */
        if (hand.isLeft()) {
            if (easyMode) {
                easyHenkei(hand);
            }
            else {
                henkei(hand);
            }
        }
        /* 回転 */
        if (hand.isRight()) {
            if (easyMode) {
                easyKaitenKakudai(hand);
            }
            else {
                kaiten(hand);
            }
            
        }
    }

    if (handList.count() == 2) {    //両手あるなら
        /* 拡大・縮小 */
        kakudai(handList);
        /* 初期化 */
        //shokika(handList);
        /* 切替 */
        //kirikae(handList);
    }

    //ジェスチャーの処理
    GestureList gestures = frame.gestures();
    for (i = 0; i < gestures.count(); i++) {
        Gesture gesture = gestures[i];

        //ジェスチャーの種類の出力
        /*
        printf("  gesture[%d] ", i);
        switch (gesture.type()) {
        case Gesture::TYPE_CIRCLE:
            printf("CIRCLE\n");
            break;
        case Gesture::TYPE_SWIPE:

            printf("SWIPE\n");
            break;
        case Gesture::TYPE_KEY_TAP:
            printf("KEY_TAP\n");
            break;
        case Gesture::TYPE_SCREEN_TAP:
            printf("SCREEN_TAP\n");
            break;
        default:
            printf("unknown\n");
            break;
        }
        */
    }

}

void SampleListener::onFocusGained(const Controller& controller) {
}

void SampleListener::onFocusLost(const Controller& controller) {
}


void CleanupExit() {
    controller.removeListener(listener);
    exit(1);
}

/***********************************
* 変形
***********************************/
void henkei(Hand hand) {
    Vector handCenter = hand.palmPosition();
    FingerList fingerList = hand.fingers();// handの指の情報を取得する

    Vector posi0, posi1;
    Vector Center = { 0,0,0 };

    double pick = 100;


    //個別の手の情報を出力する
    //printf("  hand[%d] (%6.1f,%6.1f,%6.1f), fingers:%d\n",
    // i, handCenter.x, handCenter.y, handCenter.z, fingerList.count());

    for (int j = 0; j < fingerList.count(); j++) {
        Finger finger = fingerList[j];
        Vector currentPosition = finger.tipPosition();


        if (j == 0) {
            posi0 = currentPosition / 8;
            posi0.y -= 20;
            
            if (shape == hexagon) {
                posi0.z += 0;
            }
            else {
                posi0.z += 18;
            }
        }
        else if (j == 1) {
            posi1 = currentPosition / 8;
            posi1.y -= 20;
            
            if (shape == hexagon) {
                posi1.z += 0;
            }
            else {
                posi1.z += 18;
            }
        }

        //個別の指の情報を出力する
        // printf("    finger[%d] (%6.1f,%6.1f,%6.1f)\n",
        //   j, currentPosition.x , currentPosition.y, currentPosition.z);
    }

    Center = (posi0 + posi1) / 2;
    pick = posi0.distanceTo(posi1);
    pointer = Center / 2;


    double dist = 100;
    int I =NULL, J=NULL ;   /* pointerに最も近い頂点 */

    /* pointerに最も近い頂点を探す */
    Vector tmpNearestPoint = { 0,0,0 };
    for (int i = 0; i < pointRowNum ; i++) {
        for (int j = 0; j < pointColNum; j++) {
            dist = point[i][j].distanceTo(pointer);
            if (dist < tmpNearestPoint.distanceTo(pointer)) {
                tmpNearestPoint = point[i][j];
                I = i;
                J = j;

            }
        }
    }
    dist = pointer.distanceTo(tmpNearestPoint);
   nearestPoint = tmpNearestPoint;
    
    

    if (rotatingFlag == -1 && movingFlag == -1 || movingFlag == I*100+J) {//どの点も変形中でない(-1)、または自身が変形中(j * 100 + k)
        if ( pick < 4) {
            if (movingFlag == -1) { printf("Transform Start!!\n"); }
            movingFlag = I * 100 + J;


            /* 摘まんだ点を動かす */
            printf("%d %d\n", I, J);
            point[I][J] = pointer;



            /* ポリゴンの形ごとに特殊な処理が必要な場合 */
            switch (shape) {

            case ball:
                /* 摘まんだ点の周辺も動かす */
                double TFrate;
                TFrate = point[I][J].distanceTo(prePoint[I][J]);       //摘まんだ点の変化に合わせて、周囲の点の変化率を変える
                TFrate = abs(TFrate) / (CameraDistance * 0.7);
                if (I == 0 || I == latitudeNUM) {   /* 摘まんだ点がpoleだった時 */
                    int lati1 = (I == 0) ? (1) : (latitudeNUM - 1);
                    int lati2 = (I == 0) ? (2) : (latitudeNUM - 2);
                    int lati3 = (I == 0) ? (3) : (latitudeNUM - 3);
                    for (int l = 0; l < longitudeNUM; l++) {
                        point[lati1][l] = prePoint[lati1][l] + (point[I][J] - prePoint[lati1][l]) * 0.6 * TFrate;
                        point[lati2][l] = prePoint[lati2][l] + (point[I][J] - prePoint[lati2][l]) * 0.25 * TFrate;
                        point[lati3][l] = prePoint[lati3][l] + (point[I][J] - prePoint[lati3][l]) * 0.06 * TFrate;
                    }
                }
                else {
                    /* 頂点の回りの点の動き */
                    for (int i = -3; i <= 3; i++) {
                        if (0 < I + i && I + i < latitudeNUM) {
                            for (int j = -3; j <= 3; j++) {
                                if (i != 0 || j != 0) {
                                    if (abs(i) == 3 || abs(j) == 3) {   //頂点の周辺の動き（＊頂点の隣の隣の隣）
                                        point[I + i][(J + j + longitudeNUM) % longitudeNUM] = prePoint[I + i][(J + j + longitudeNUM) % longitudeNUM] + (point[I][J] - prePoint[I + i][(J + j + longitudeNUM) % longitudeNUM]) * 0.06 * TFrate;
                                    }
                                    else if (abs(i) == 2 || abs(j) == 2) {  //頂点の周辺の動き（＊頂点の隣の隣）
                                        point[I + i][(J + j + longitudeNUM) % longitudeNUM] = prePoint[I + i][(J + j + longitudeNUM) % longitudeNUM] + (point[I][J] - prePoint[I + i][(J + j + longitudeNUM) % longitudeNUM]) * 0.26 * TFrate;
                                    }
                                    else {  //頂点の周辺の動き（＊頂点の隣）
                                        point[I + i][(J + j + longitudeNUM) % longitudeNUM] = prePoint[I + i][(J + j + longitudeNUM) % longitudeNUM] + (point[I][J] - prePoint[I + i][(J + j + longitudeNUM) % longitudeNUM]) * 0.6 * TFrate;
                                    }

                                }
                            }
                        }

                    }
                }
                break;

            case cube:

                break;

            case hexagon:
                break;
            default:
                break;

            }

        }
        else {
            if (pick >= 4) {
                if (movingFlag == 0) { printf("Transform Finished!!!\n"); }
                movingFlag = -1;
            }

        }
        
    }
    else {
        if (pick >= 4) {
            movingFlag = -1;
        }

    }
}

/***********************************
* 回転
***********************************/
void kaiten(Hand hand) {
    Vector handCenter = hand.palmPosition();
    FingerList fingerList = hand.fingers();// handの指の情報を取得する

    Vector posi0, posi1;
    Vector Center = { 0,0,0 };

    double pick = 100;


    //個別の手の情報を出力する
    //printf("  hand[%d] (%6.1f,%6.1f,%6.1f), fingers:%d\n",
    // i, handCenter.x, handCenter.y, handCenter.z, fingerList.count());
    for (int j = 0; j < fingerList.count(); j++) {
        Finger finger = fingerList[j];
        Vector currentPosition = finger.tipPosition();


        if (j == 0) {
            posi0 = currentPosition / 8;
            posi0.y -= 30;
            posi0.z += 8;
        }
        else if (j == 1) {
            posi1 = currentPosition / 8;
            posi1.y -= 30;
            posi1.z += 8;
        }

        //個別の指の情報を出力する
        // printf("    finger[%d] (%6.1f,%6.1f,%6.1f)\n",
        //   j, currentPosition.x , currentPosition.y, currentPosition.z);
    }

    Center = (posi0 + posi1) / 2;
    pick = posi0.distanceTo(posi1);

    rotateStart = (rotateNow.y == 999) ? Center : rotateStart;
    rotateNow = Center;
    if (movingFlag == -1 && pick < 4 || rotatingFlag == 1) {
        if (rotatingFlag == -1) { printf("Rotation Start!!!\n"); }
        rotatingFlag = 1;
        for (int j = 0; j < pointRowNum; j++) {
            for (int k = 0; k < pointColNum; k++) {
                /* 移動量を計算する */
                Vector Move = rotateNow - rotateStart;


                ////* x軸周りに回転 *////
                double RotateX = (point[j][k].z == 0) ? PI / 2 * sign(point[j][k].y) : atan((point[j][k].y / abs(point[j][k].z)));    /* 頂点の位置（x軸周りの角度） */
                int ZMark = (point[j][k].z < 0) ? -1 : 1;     /* zの正負 */
                double XDistance = (double)pow(point[j][k].y * point[j][k].y + point[j][k].z * point[j][k].z, 0.5);     /* x軸までの距離 */

                /* Moveのx軸方向のモーメントから回転方向と回転量を計算する */
                Vector XAxisPerpendicular = { 0,rotateStart.y,rotateStart.z };
                Vector XMove = { 0,Move.y,Move.z };
                Vector XMoment = -XAxisPerpendicular.cross(XMove);
                RotateX += (ZMark < 0) ? -0.02 * XMoment.x : 0.02 * XMoment.x;


                /* 回転させる */
                point[j][k].y = XDistance * sin(RotateX);
                point[j][k].z = XDistance * cos(RotateX) * ZMark;



                ////* y軸周りに回転 *////
                double RotateY = (point[j][k].x == 0) ? PI / 2 * sign(point[j][k].x) : atan((point[j][k].z / abs(point[j][k].x)));    /* 頂点の位置（y軸周りの角度） */
                int XMark = (point[j][k].x < 0) ? -1 : 1;     /* ⅹの正負 */
                double YDistance = (double)pow(point[j][k].z * point[j][k].z + point[j][k].x * point[j][k].x, 0.5);     /* y軸までの距離 */

                /* Moveのy軸方向のモーメントから回転方向と回転量を計算する */
                Vector YAxisPerpendicular = { rotateStart.x,0,rotateStart.z };
                Vector YMove = { Move.x,0,Move.z };
                Vector YMoment = -YAxisPerpendicular.cross(YMove);
                RotateY += (XMark < 0) ? -0.02 * YMoment.y : 0.02 * YMoment.y;

                /* 回転させる */
                point[j][k].z = YDistance * sin(RotateY);
                point[j][k].x = YDistance * cos(RotateY) * XMark;



                ////* z軸周りに回転 *////
                double RotateZ = (point[j][k].y == 0) ? PI / 2 * sign(point[j][k].y) : atan((point[j][k].x / abs(point[j][k].y)));    /* 頂点の位置（z軸周りの角度） */
                int YMark = (point[j][k].y < 0) ? -1 : 1;     /* yの正負 */
                double ZDistance = (double)pow(point[j][k].x * point[j][k].x + point[j][k].y * point[j][k].y, 0.5);     /* z軸までの距離 */

                /* Moveのz軸方向のモーメントから回転方向と回転量を計算する */
                Vector ZAxisPerpendicular = { rotateStart.x,rotateStart.y,0 };
                Vector ZMove = { Move.x,Move.y,0 };
                Vector ZMoment = ZAxisPerpendicular.cross(ZMove);
                RotateZ += (YMark < 0) ? 0.02 * ZMoment.z : -0.02 * ZMoment.z;

                /* 回転させる */

                point[j][k].x = ZDistance * sin(RotateZ);
                point[j][k].y = ZDistance * cos(RotateZ) * YMark;



                if (j == 0 && k == 0) {

                    printf("Rotate X:%.1f, Y:%.1f, Z:%.1f\n", XMoment.x, XMoment.y, XMoment.z);
                    printf("Rotate X:%.1f, Y:%.1f, Z:%.1f\n", YMoment.x, YMoment.y, YMoment.z);
                    printf("Rotate X:%.1f, Y:%.1f, Z:%.1f\n\n", ZMoment.x, ZMoment.y, ZMoment.z);
                }


            }
        }
    }
    if (pick >= 4 && rotatingFlag == 1) {
        rotatingFlag = -1;
        printf("Rotation Finished!!\n");
    }
    /* 次回の回転用に更新 */
    if (hand.isRight()) {
        rotateStart = rotateNow;
    }

}

/***********************************
* 拡大・縮小
***********************************/
void kakudai(HandList handList) {
    Hand handR = (handList[0].isRight()) ? handList[0] : handList[1];
    Hand handL = (handList[0].isLeft()) ? handList[0] : handList[1];
    FingerList RfingerList = handR.fingers();
    FingerList LfingerList = handL.fingers();

    Vector Rposi0 = RfingerList[0].tipPosition() / 8;       //右親指
    Vector Lposi0 = LfingerList[0].tipPosition() / 8;       //左親指
    Vector Rposi1 = RfingerList[1].tipPosition() / 8;       //右人差し指
    Vector Lposi1 = LfingerList[1].tipPosition() / 8;       //左人差し指
    Vector Rposi2 = RfingerList[2].tipPosition() / 8;       //右中指
    Vector Lposi2 = LfingerList[2].tipPosition() / 8;       //左中指
    Vector Rposi3 = RfingerList[3].tipPosition() / 8;       //右薬指
    Vector Lposi3 = LfingerList[3].tipPosition() / 8;       //左薬指
    Vector Rposi4 = RfingerList[4].tipPosition() / 8;       //右小指
    Vector Lposi4 = LfingerList[4].tipPosition() / 8;       //左小指

    double scale = Rposi1.distanceTo(Lposi1);    //右人差し指と左人差し指の距離
        /* 「手のひらを合わせた」かつ「頂点を動かしていない」かつ「回転してない」時に大きさを調整 */
    if (scale < 4 && movingFlag == -1 && rotatingFlag == -1 || scalingFlag == 1) {
        if (scalingFlag == -1) printf("scaling start!!!\n");
        movingFlag = -2;
        rotatingFlag = -2;
        scalingFlag = 1;




        for (int i = 0; i < pointRowNum; i++) {
            for (int j = 0; j < pointColNum; j++) {
                point[i][j] = prePoint[i][j] * 0.1 * scale;
            }
        }
        printf("magnification：%f\n", 0.1 * scale);

        double finishR = Rposi1.distanceTo(Rposi0);     //右人差し指と右親指の距離
        double finishL = Lposi1.distanceTo(Lposi0);     //左人差し指と左親指の距離
        if (finishR < 4 || finishL < 4) {      //摘まむしぐさでスケーリング終了
            movingFlag = -1;
            rotatingFlag = -1;
            scalingFlag = -1;
            printf("scaling finished!\n");
        }
    }
}
/***********************************
* 初期化
***********************************/
void shokika(HandList handList) {
    Hand handR = (handList[0].isRight()) ? handList[0] : handList[1];
    Hand handL = (handList[0].isLeft()) ? handList[0] : handList[1];
    FingerList RfingerList = handR.fingers();
    FingerList LfingerList = handL.fingers();

    Vector Rposi0 = RfingerList[0].tipPosition() / 8;       //右親指
    Vector Lposi0 = LfingerList[0].tipPosition() / 8;       //左親指
    Vector Rposi1 = RfingerList[1].tipPosition() / 8;       //右人差し指
    Vector Lposi1 = LfingerList[1].tipPosition() / 8;       //左人差し指
    Vector Rposi2 = RfingerList[2].tipPosition() / 8;       //右中指
    Vector Lposi2 = LfingerList[2].tipPosition() / 8;       //左中指
    Vector Rposi3 = RfingerList[3].tipPosition() / 8;       //右薬指
    Vector Lposi3 = LfingerList[3].tipPosition() / 8;       //左薬指
    Vector Rposi4 = RfingerList[4].tipPosition() / 8;       //右小指
    Vector Lposi4 = LfingerList[4].tipPosition() / 8;       //左小指


    /* 手が重なると、正常に動かないので二次元の距離を考える */
    Rposi3.z = Lposi1.z;
    Lposi3.z = Rposi1.z;


    double init[2] = { Rposi1.distanceTo(Lposi3),Lposi1.distanceTo(Rposi3) };//人差し指と薬指の距離

    if (movingFlag == -1 && rotatingFlag == -1 && scalingFlag == -1) { //どの操作もしていないなら

        if (init[0] < 4 || init[1] < 4) {   //薬指と人差し指が近づいたら
            printf("initialize!!!");
            movingFlag = -3;
            rotatingFlag = -3;
            scalingFlag = -3;
            switch (shape)
            {
            case ball:
                InitBall();
                break;
            case cube:
                InitCube();
                break;
            case hexagon:
                InitHexagon();
                break;
            default:
                break;
            }
            Sleep(800);
            movingFlag = -1;
            rotatingFlag = -1;
            scalingFlag = -1;
        }
    }

}
void shokika() {

    if (movingFlag == -1 && rotatingFlag == -1 && scalingFlag == -1) { //どの操作もしていないなら

        printf("initialize!!!");
        movingFlag = -3;
        rotatingFlag = -3;
        scalingFlag = -3;
        switch (shape)
        {
        case ball:
            InitBall();
            break;
        case cube:
            InitCube();
            break;
        case hexagon:
            InitHexagon();
            break;
        default:
            break;
        }
        Sleep(800);
        movingFlag = -1;
        rotatingFlag = -1;
        scalingFlag = -1;
    }

}
/***********************************
* 切替
***********************************/
void kirikae(HandList handList) {
    Hand handR = (handList[0].isRight()) ? handList[0] : handList[1];
    Hand handL = (handList[0].isLeft()) ? handList[0] : handList[1];
    FingerList RfingerList = handR.fingers();
    FingerList LfingerList = handL.fingers();

    Vector Rposi0 = RfingerList[0].tipPosition() / 8;       //右親指
    Vector Lposi0 = LfingerList[0].tipPosition() / 8;       //左親指
    Vector Rposi1 = RfingerList[1].tipPosition() / 8;       //右人差し指
    Vector Lposi1 = LfingerList[1].tipPosition() / 8;       //左人差し指
    Vector Rposi2 = RfingerList[2].tipPosition() / 8;       //右中指
    Vector Lposi2 = LfingerList[2].tipPosition() / 8;       //左中指
    Vector Rposi3 = RfingerList[3].tipPosition() / 8;       //右薬指
    Vector Lposi3 = LfingerList[3].tipPosition() / 8;       //左薬指
    Vector Rposi4 = RfingerList[4].tipPosition() / 8;       //右小指
    Vector Lposi4 = LfingerList[4].tipPosition() / 8;       //左小指


    /* 手が重なると、正常に動かないので二次元の距離を考える */
    Rposi4.z = Lposi1.z;
    Lposi4.z = Rposi1.z;

    double swichPolygon[2] = { Rposi1.distanceTo(Lposi4),Lposi1.distanceTo(Rposi4) };//人差し指と薬指の距離

    if (movingFlag == -1 && rotatingFlag == -1 && scalingFlag == -1) { //どの操作もしていないなら
        if (swichPolygon[0] < 4 || swichPolygon[1] < 4) {   //薬指と人差し指が近づいたら
            printf("switch!!!");
            movingFlag = -1;
            rotatingFlag = -1;
            scalingFlag = -1;
            switch (shape)
            {
            case ball:
                InitCube();
                break;
            case cube:
                InitHexagon();
                break;
            case hexagon:
                InitBall();
                break;
            default:
                break;
            }
            Sleep(800);
            movingFlag = -1;
            rotatingFlag = -1;
            scalingFlag = -1;
        }
    }
}
void kirikae(int i) {

    
    if (movingFlag == -1 && rotatingFlag == -1 && scalingFlag == -1) { //どの操作もしていないなら
        printf("switch!!!");
        movingFlag = -1;
        rotatingFlag = -1;
        scalingFlag = -1;

        if (i <0) {
            int tmp = (int)shape;
            tmp -= 2;
            shape = (polygon)tmp;
        }
        
        switch (shape)
        {
        case ball:
            InitCube();
            break;
        case cube:
            InitHexagon();
            break;
        case hexagon:
            InitBall();
            break;
        default:
            break;
        }
        Sleep(800);
        movingFlag = -1;
        rotatingFlag = -1;
        scalingFlag = -1;
    }
}

/**************************************
* 簡単モード関数
**************************************/
void easyHenkei(Hand hand) {
    Vector handCenter = hand.palmPosition();
    FingerList fingerList = hand.fingers();// handの指の情報を取得する

    Vector posi0, posi1;
    Vector Center = { 0,0,0 };

    double pick = 100;


    //個別の手の情報を出力する
    //printf("  hand[%d] (%6.1f,%6.1f,%6.1f), fingers:%d\n",
    // i, handCenter.x, handCenter.y, handCenter.z, fingerList.count());

    for (int j = 0; j < fingerList.count(); j++) {
        Finger finger = fingerList[j];
        Vector currentPosition = finger.tipPosition();


        if (j == 0) {
            posi0 = currentPosition / 8;
            posi0.y -= 20;

            if (shape == hexagon) {
                posi0.z += 0;
            }
            else {
                posi0.z += 18;
            }
        }
        else if (j == 1) {
            posi1 = currentPosition / 8;
            posi1.y -= 20;

            if (shape == hexagon) {
                posi1.z += 0;
            }
            else {
                posi1.z += 18;
            }
        }

        //個別の指の情報を出力する
        // printf("    finger[%d] (%6.1f,%6.1f,%6.1f)\n",
        //   j, currentPosition.x , currentPosition.y, currentPosition.z);
    }

    Center = (posi0 + posi1) / 2;
    pick = posi0.distanceTo(posi1);
    pointer = Center / 2;


    double dist = 100;
    int I = NULL, J = NULL;   /* pointerに最も近い頂点 */

    /*  */

    double scale = 1 + (pointer.z - prePointer.z) / 30;

    printf("scale::%f\n", scale);

    if (rotatingFlag == -1 && movingFlag == -1 || movingFlag == 9999) {//どの点も変形中でない(-1)、または自身が変形中(j * 100 + k)
        if (pick < 4) {
            if (movingFlag == -1) { printf("Transform Start!!\n"); }
            movingFlag = 9999;

            for (int I = 0; I < pointRowNum; I++) {
                for (int J = 0; J < pointColNum; J++) {

                    /* 点を動かす */
                    printf("%d %d\n", I, J);
                    if ((I + J) % 2 && I % 2) {

                        point[I][J] = prePoint[I][J] * scale;
                    }


                    /* ポリゴンの形ごとに特殊な処理が必要な場合 */


                }
            }


        }
        else {
            if (pick >= 4) {
                if (movingFlag == 0) { printf("Transform Finished!!!\n"); }
                movingFlag = -1;
            }

        }

    }
    else {
        if (pick >= 4) {
            movingFlag = -1;
        }

    }
}
void easyKaitenKakudai(Hand hand) {
    Vector handCenter = hand.palmPosition();
    FingerList fingerList = hand.fingers();// handの指の情報を取得する

    Vector posi0, posi1;
    Vector Center = { 0,0,0 };

    double pick = 100;


    //個別の手の情報を出力する
    //printf("  hand[%d] (%6.1f,%6.1f,%6.1f), fingers:%d\n",
    // i, handCenter.x, handCenter.y, handCenter.z, fingerList.count());
    for (int j = 0; j < fingerList.count(); j++) {
        Finger finger = fingerList[j];
        Vector currentPosition = finger.tipPosition();


        if (j == 0) {
            posi0 = currentPosition / 8;
            posi0.y -= 30;
            posi0.z += 8;
        }
        else if (j == 1) {
            posi1 = currentPosition / 8;
            posi1.y -= 30;
            posi1.z += 8;
        }

        //個別の指の情報を出力する
        // printf("    finger[%d] (%6.1f,%6.1f,%6.1f)\n",
        //   j, currentPosition.x , currentPosition.y, currentPosition.z);
    }

    Center = (posi0 + posi1) / 2;
    pick = posi0.distanceTo(posi1);
    pointer = Center / 2;

    rotateStart = (rotateNow.y == 999) ? Center : rotateStart;
    rotateNow = Center;
    

    if (movingFlag == -1 && pick < 4 || rotatingFlag == 1) {
        if (rotatingFlag == -1) { printf("Rotation Start!!!\n"); }
        rotatingFlag = 1;
        double scale = 1 + (pointer.y - prePointer.y) / 30;
        /* 移動量を計算する */
        double MoveX = pointer.x - prePointer.x;
        for (int j = 0; j < pointRowNum; j++) {
            for (int k = 0; k < pointColNum; k++) {

                Vector kakudaiPrePoint = scale * prePoint[j][k];

                ////* y軸周りに回転 *////
                double RotateY = (kakudaiPrePoint.x == 0) ? PI / 2 * sign(kakudaiPrePoint.x) : atan((kakudaiPrePoint.z / abs(kakudaiPrePoint.x)));    /* 頂点の位置（y軸周りの角度） */
                int XMark = (kakudaiPrePoint.x < 0) ? -1 : 1;     /* ⅹの正負 */
                double YDistance = (double)pow(kakudaiPrePoint.z * kakudaiPrePoint.z + kakudaiPrePoint.x * kakudaiPrePoint.x, 0.5);     /* y軸までの距離 */

                RotateY -= 0.1 * XMark * MoveX ;

                /* 回転させる */
                point[j][k].y = kakudaiPrePoint.y;
                point[j][k].z = YDistance * sin(RotateY);
                point[j][k].x = YDistance * cos(RotateY) * XMark;


                



            }
        }
    }

    

    if (pick >= 4 && rotatingFlag == 1) {
        rotatingFlag = -1;
        printf("Rotation Finished!!\n");
    }
    /* 次回の回転用に更新 */
    if (hand.isRight()) {
        rotateStart = rotateNow;
    }

}