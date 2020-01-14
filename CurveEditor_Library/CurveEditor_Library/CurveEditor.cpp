#include "stdafx.h"
#include "CurveEditor.h"
#include <fstream>
#include <cmath>
#include <Windows.h>
//#include <iostream>
//#include <sstream>
namespace CurveEditor
{

	/* 小数点n以下で四捨五入する */
	double round_n(double number, double n)
	{
		double num = pow(10.0, n - 1);
		number = number *num; //四捨五入したい値を10の(n-1)乗倍する。
		number = round(number); //小数点以下を四捨五入する。
		number /= num; //10の(n-1)乗で割る。
		return number;
	}

	BezierPointList::BezierPointList()
		:Ax(0),
		Ay(0),
		Bx(0),
		By(0),
		Cx(0),
		Cy(0)
	{

	}

	BezierPointList::~BezierPointList()
	{

	}
	//CSVからグラフデータを読み込む
	bool BezierPointList::ReadBezierPointList(std::string CSVpath)
	{
		//念のためデータを開放
		Release();

		std::ifstream ifs;
		ifs.open(CSVpath.c_str());

		std::string value;
		std::vector<std::string > values;

		bool isOpen = ifs.is_open();//正常なデータを開けたか

		if (isOpen)
		{
			//一行読み込みテスト
		//	std::getline(ifs, value);
		//	if (value != "/n") values.push_back(value);
			//全データを読み込む
			while (std::getline(ifs, value, ','))
			{
				if(value != "/n") values.push_back(value);
			}
			//データを保存
			StringToBezierPoint(values);

			ifs.close();
			if (!DateErrorCheck())
			{
#ifdef _DEBUG
			OutputDebugStringA("CurveEditoreError[000][CSVのデータがおかしいよ！] \n");
#endif // DEBUG
				return false;
			}
		}
		else
		{
#ifdef _DEBUG
			OutputDebugStringA("CurveEditoreError[001][ファイルを開けてないよ！] \n");
#endif // DEBUG
		}

		return isOpen;
	}
	// グラフのXからYを求める
	double BezierPointList::EvaluateY(double x)
	{
		int num = 0;
		int pontcnt = m_List.size();
		//どこのベジェ曲線か検索
		num = SearchBezier(x);

		m_SelectPoint = m_List[num];

		return CalcYfromX(x);
	}
	//グラフデータに異常はないか？　trueなら正常
	bool BezierPointList::DateErrorCheck()
	{
		//グラフデータがおかしいかの判定
		//データの数値チェック
		if (m_List.size() <= 0) return false;
		if (m_List[0].startPoint.x != 0) return false;
		if (m_List[m_List.size() - 1].endPoint.x != 1) return false;;

		return true;
	}

	int BezierPointList::SearchBezier(double x)
	{
		int num = 0;
		int pontcnt = m_List.size();
		//どこのベジェ曲線か検索
		for (int i = 0; i < pontcnt; i++)
		{
			if (m_List[i].endPoint.x > x)
			{
				num = i;
				return num;
			}
		}
		// 0～1の間でない値なら端の曲線を返す
		if (x >= 1) return pontcnt - 1;
		if (x <= 0) return  0;
		return 0;
	}

	void BezierPointList::SetConstants3()
	{
		Vec2 b0(m_SelectPoint.startPoint.x, m_SelectPoint.startPoint.y) ;
		Vec2 b1(m_SelectPoint.controlPoint1.x, m_SelectPoint.controlPoint1.y);
		Vec2 b2(m_SelectPoint.controlPoint2.x, m_SelectPoint.controlPoint2.y);
		Vec2 b3(m_SelectPoint.endPoint.x, m_SelectPoint.endPoint.y);

		//公式に当てはめる
		/*ベジェ曲線は媒介変数表示で定義される。tを0～1で変化させると、曲線上の点を取得できる。
		(x1,y1)始点
		(x2,y2)制御点
		(x3,y3)制御点
		(x4,y4)終点

		x = x(t) = t^3*x4 + 3*t^2*(1-t)*x3 + 3*t*(1-t)^2*x2 + (1-t)^3*x1
		y = y(t) = t^3*y4 + 3*t^2*(1-t)*y3 + 3*t*(1-t)^2*y2 + (1-t)^3*y1

		tについて降べきの順に整理すると

		x = (x4-3*(x3+x2)-x1)*t^3 + 3*(x3-2*x2+x1)*t^2 + 3*(x2-x1)*t + x1
		y = (y4-3*(y3+y2)-y1)*t^3 + 3*(y3-2*y2+y1)*t^2 + 3*(y2-y1)*t + y1

		t以外の値を求める?
		*/
		Ax = b3.x - 3 * (b2.x - b1.x) - b0.x;
		Bx = 3 * (b2.x - 2 * b1.x + b0.x);
		Cx = 3 * (b1.x - b0.x);

		Ay = b3.y - 3 * (b2.y - b1.y) - b0.y;
		By = 3 * (b2.y - 2 * b1.y + b0.y);
		Cy = 3 * (b1.y - b0.y);
	}
	double BezierPointList::CalcYfromX(double x)
	{
		double epsilon = 0.001f; // 閾値
		double x2, t0, t1, t2, d2, i;
		for (t2 = x, i = 0; i < 8; i++)
		{
			x2 = GetPointAtTime(t2).x - x;
			if (abs(x2) < epsilon)
			{
				return GetPointAtTime(t2).y;
			}
			d2 = sampleCurveDerivativeX(t2);
			if (abs(d2) < 1e-6f)
			{
				break;
			}
			t2 = t2 - x2 / d2;
		}
		t0 = 0;
		t1 = 1;
		t2 = x;
		//tが0～1の間でないなら
		if (t2 < t0)
		{
			return GetPointAtTime(t0).y;
		}
		//tが0～1の間でないなら
		if (t2 > t1)
		{
			return GetPointAtTime(t1).y;
		}
		while (t0 < t1)
		{
			x2 = GetPointAtTime(t2).x;
			if (abs(x2 - x) < epsilon)
			{
				return GetPointAtTime(t2).y;
			}
			if (x > x2)
			{
				t0 = t2;
			}
			else
			{
				t1 = t2;
			}
			t2 = (t1 - t0) * 0.5f + t0;
		}

		return GetPointAtTime(t2).y; // 失敗
	}
	// あるtについてxの微分値を求める
	double BezierPointList::sampleCurveDerivativeX(double t)
	{
		// //3次ベジェ
		return (3.0f * Ax * t + 2.0f * Bx) * t + Cx;
	}
	// tからxとyを求める
	BezierPointList::Vec2 BezierPointList::GetPointAtTime(double t)
	{
		SetConstants3();
		//参考プログラムのやつ　こっちのほうが計算が早いかも？
		/* Vec2 p0 = new Vec2((float)m_SelectPoint.startPoint[0], (float)m_SelectPoint.startPoint[1]);
		float t2 = t * t;
		float t3 = t * t * t;
		float x = this.Ax * t3 + this.Bx * t2 + this.Cx * t + p0.x;
		float y = this.Ay * t3 + this.By * t2 + this.Cy * t + p0.y;
		*/

		double x1 = m_SelectPoint.startPoint.x;
		double y1 = m_SelectPoint.startPoint.y;
		double x2 = m_SelectPoint.controlPoint1.x;
		double y2 = m_SelectPoint.controlPoint1.y;
		double x3 = m_SelectPoint.controlPoint2.x;
		double y3 = m_SelectPoint.controlPoint2.y;
		double x4 = m_SelectPoint.endPoint.x;
		double y4 = m_SelectPoint.endPoint.y;

		double t1 = t;
		double t2 = t1 * t1;
		double t3 = t1 * t1 * t1;
		double tp1 = 1 - t1;
		double tp2 = tp1 * tp1;
		double tp3 = tp1 * tp1 *tp1;
		//公式に当てはめる
		/*ベジェ曲線は媒介変数表示で定義される。tを0～1で変化させると、曲線上の点を取得できる。
		(x1,y1)始点
		(x2,y2)制御点
		(x3,y3)制御点
		(x4,y4)終点

		x = x(t) = t^3*x4 + 3*t^2*(1-t)*x3 + 3*t*(1-t)^2*x2 + (1-t)^3*x1
		y = y(t) = t^3*y4 + 3*t^2*(1-t)*y3 + 3*t*(1-t)^2*y2 + (1-t)^3*y1

		媒介変数[t]を用いてるのでxとyの両方の解がでる
		*/

		double tx = (tp3 * x1) + (x2 * 3 * tp2 * t1) + (x3 * 3 * tp1 * t2) + (t3 * x4);
		double ty = (tp3 * y1) + (y2 * 3 * tp2 * t1) + (y3 * 3 * tp1* t2) + (t3 * y4);


		return  Vec2(tx, ty);
	}
	//文字列から数字に変換しデータを保存
	void BezierPointList::StringToBezierPoint(std::vector<std::string>& val)
	{
		int loopcnt = val.size() / 8;
		for (int i = 0; i < loopcnt; i++)
		{
			//文字列を数字に変換
			Vec2 s(std::stod(val[i * 8]), std::stod(val[1 + i * 8]));		//開始点
			Vec2 c1(std::stod(val[ 2 +i * 8]), std::stod(val[3  + i * 8]));	//制御点1
			Vec2 c2(std::stod(val[4 + i * 8]), std::stod(val[5  + i * 8]));	//制御点２
			Vec2 e(std::stod(val[6 + i * 8]), std::stod(val[7  + i * 8]));	//終了点
			//保存
			BezierPoint bp(s, c1, c2, e);
			m_List.push_back(bp);
		}
	}
	void BezierPointList::Release()
	{
		std::vector<BezierPoint> List;
		List.swap(m_List);
		m_List.clear();

	}
}