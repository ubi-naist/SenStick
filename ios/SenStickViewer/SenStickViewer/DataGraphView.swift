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
    
    var value: [[Double]] = [[],[],[]]
    
    // 内部的に、300点までのパスを保持する
    let pathCount :Int = 300
    var pathData :[[CGFloat]] = []
    let pathColor :[CGColor]  = [UIColor.red.cgColor, UIColor.green.cgColor, UIColor.blue.cgColor]

    func clearPlot()
    {
        pathData = []
        setNeedsDisplay()
    }
    
    // データを描画します 配列は、[x, y, z]を想定。x, もしくはxおよびyのみの利用も可能。
    func plotData(_ value:[[Double]])
    {
        self.value = value
        
        // 再描画
        setNeedsDisplay()
    }
    
    override func draw(_ rect: CGRect)
    {
        super.draw(rect)

        // パスデータを構築
        pathData = [[], [], []]
        // データを間引きながら追加します, 配列の中の配列はそれぞれ軸のデータを表す。
        for (axisIndex, dataArray) in value.enumerated() {
            for i in (0..<dataArray.count) {
                var canAdd = false
                if sampleCount <= pathCount {
                    // そのままデータを追加してOK
                    canAdd = true
                } else {
                    // データ点列をsampleCountの値に合わせて、間引くなりする
                    nextSamplePoint += Double(pathCount) / Double(sampleCount)
                    if nextSamplePoint > 1 {
                        nextSamplePoint -= 1
                        canAdd = true
                    }
                }
                // データを正規化して追加
                if canAdd {
                    let y = CGFloat((dataArray[i] - minValue) / (maxValue - minValue))
                    assert(!y.isNaN)
                    assert(!y.isInfinite)
                    pathData[axisIndex].append(y)
                }
            }
        }
        
        // 描画
        let context = UIGraphicsGetCurrentContext()!
        
        for (index, apath) in pathData.enumerated() {
            if apath.count == 0 {
                continue
            }
            // 1つのパスを描画する
            let drawPath = CGMutablePath()
            let dx = self.frame.width / CGFloat(pathCount)
            for i in 0..<apath.count {
                var y = apath[i] * self.frame.height
                // iOSの描画系は左上が原点なので、y軸を反転する
                y = self.frame.height - y
                let x = CGFloat(i) * dx
                assert(!x.isNaN)
                assert(!y.isNaN)
                if i == 0 {
                    drawPath.move(to: CGPoint(x: x, y: y))
                } else {
                    drawPath.addLine(to: CGPoint(x: x, y: y))
                }
            }
            // パスを追加
            context.addPath(drawPath)
            context.setLineWidth(2)
            context.setStrokeColor(pathColor[index])
            context.strokePath()
        }
    }
}
