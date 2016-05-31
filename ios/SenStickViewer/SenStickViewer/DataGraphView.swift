//
//  graphView.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/24.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class DataGraphView : UIView {

    // Y軸の最大/最小
    var maxValue : Double = 1.0
    var minValue : Double = 0.0

    // データを追加するときの、今まで追加したデータのカウント
    var nextSamplePoint: Double = 0
    // 1グラフに収めるサンプルカウント数
    var sampleCount: Int = 300 {
        didSet {
            nextSamplePoint = 0
        }
    }
    
    // サンプルカウント数が描画幅を超えたら最初から描画し直すか?
    var autoRedraw: Bool = true
    
    // 内部的に、300点までのパスを保持する
    let pathCount :Int = 300
    var pathData :[[CGFloat]] = [[], [], []]
    let pathColor :[CGColor]  = [UIColor.redColor().CGColor, UIColor.greenColor().CGColor, UIColor.blueColor().CGColor]
    
    // データを追加します, 配列はデータの種類ごとに並んだ1サンプル。
    private func addData(value:[Double]) {
        // パスをフラッシュ
        if pathData[0].count > pathCount && autoRedraw {
            pathData = [[], [], []]
        }

        // データを間引きながら追加
        var data: [Double] = []
        if sampleCount <= pathCount {
            // そのままデータを追加してOK
            data = value
        } else {
            // データ点列をsampleCountの値に合わせて、間引くなりする
            nextSamplePoint += Double(pathCount / sampleCount)
            if nextSamplePoint > 1 {
                nextSamplePoint -= 1
                data = value
            }
        }
        
        // データ追加がないなら終了
        if data.count == 0 {
            return
        }
        
        // データを正規化して追加
        for (index, d) in data.enumerate() {
            let y = CGFloat((d - minValue) / (maxValue - minValue))
            pathData[index].append(y)
        }
    }

    func clearPlot()
    {
        pathData = [[], [], []]
        setNeedsDisplay()
    }
    
    func plotData(value:[Double])
    {
        addData(value)
        setNeedsDisplay()
    }
    
    override func drawRect(rect: CGRect)
    {
        super.drawRect(rect)

        let context = UIGraphicsGetCurrentContext()!
        
        for (index, apath) in pathData.enumerate() {
            if apath.count == 0 {
                continue
            }
            // 1つのパスを描画する
            let drawPath = CGPathCreateMutable()
            let dx = self.frame.width / CGFloat(pathCount)
            for i in 0..<apath.count {
                var y = apath[i] * self.frame.height
                // iOSの描画系は左上が原点なので、y軸を反転する
                y = self.frame.height - y
                if i == 0 {
                    CGPathMoveToPoint(drawPath, nil, CGFloat(i) * dx, y)
                } else {
                    CGPathAddLineToPoint(drawPath, nil, CGFloat(i) * dx, y)
                }
            }
            // パスを追加
            CGContextAddPath(context, drawPath)
            CGContextSetLineWidth(context, 2)
            CGContextSetStrokeColorWithColor(context, pathColor[index])
            CGContextStrokePath(context)
        }
    }
}
