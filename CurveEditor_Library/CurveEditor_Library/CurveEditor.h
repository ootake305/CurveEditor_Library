#pragma once

#include<vector>
#include <string>


namespace CurveEditor
{
	 //CSVからグラフデータを読み込み、保存やデータを読み込むクラス
	 class BezierPointList
	 {
	 public:
		 struct Vec2
		 {
			 double x;
			 double y;

			 Vec2()
			 {

			 }
			 Vec2(double x1, double y2)
			 {
				 x = x1;
				 y = y2;
			 }
		 };
		 //3次ベジェ曲線に必要な点
		 struct BezierPoint
		 {
			 Vec2 startPoint;     //開始点
			 Vec2 controlPoint1;  //制御点1
			 Vec2 controlPoint2;  //制御点2
			 Vec2 endPoint;       //終了点

			 BezierPoint()
			 {

			 }
			 BezierPoint(Vec2 s, Vec2 c1, Vec2 c2, Vec2 e)
			 {
				 startPoint = s;
				 controlPoint1 = c1;
				 controlPoint2 = c2;
				 endPoint = e;
			 }
		 };
	 public:
		 BezierPointList();
		  ~BezierPointList();

		 //CSVからグラフデータを読み込む
		 // @param Actor　読み込むCSVのパス
		 // @return  difference 誤差の許容値
		 bool ReadBezierPointList(std::string CSVpath);

		 // グラフのXからYを求める
		 // @param x　時間0～1

		 double EvaluateY(double x);

		 //グラフデータに異常はないか？
		 // @return  true なら正常
		 bool DateErrorCheck();
	 private:
		 //方程式を求める時に使う
		  double Ax;
		  double Ay;
		  double Bx;
		  double By;
		  double Cx;
		  double Cy;

		  std::vector<BezierPoint> m_List;
		  BezierPoint m_SelectPoint;
	 private:
		 BezierPointList(const BezierPointList & cpy);
		 BezierPointList& operator=(const BezierPointList &ope);
		 //どこの曲線か検索
		 // @param x　時間0～1
		 int SearchBezier(double x);

		 void SetConstants3();
		 // xを与えるとtについての方程式を求めてyを返す
		 // @param x　
		 double CalcYfromX(double x);
		 // あるtについてxの微分値を求める
		 // @param t
		 double sampleCurveDerivativeX(double t);
		 // tからxとyを求める
		 Vec2 GetPointAtTime(double t);

		 //文字列から数字に変換しデータを保存
		 void  StringToBezierPoint(std::vector<std::string>& val);

		 //グラフデータの開放
		 void Release();

	 };
}
