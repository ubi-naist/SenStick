//
//  graphView.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/24.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK
/*
 struct graphData {
 var color    :UIColor
 var data     :[Double]
 var maxValue :Double
 var minValue :Double
 var drawArea :Double
 }*/

class DataGraphView : UIView {
    
    var maxValue : Double = 1.0
    var minValue : Double = 0.0
    
    let maxIndex :Int = 150
    
    var data: [[Double]] = [[], [], []]
    var drawPercent: CGFloat = 1.0
    
    var drawColor: [CGColor] = [UIColor.redColor().CGColor, UIColor.greenColor().CGColor, UIColor.blueColor().CGColor]
    
    func plotData(value:Double) {
        plotDataArray([value])
    }
    
    func plotDataArray(value:[Double]) {
        for (index, d) in value.enumerate() {
            data[index].append(d)
        }

        // 描画可能サイズを超えていたらクリア
        if data[0].count > maxIndex {
            data = [[], [], []]
        }
        
        drawPercent = CGFloat(data[0].count) / CGFloat(maxIndex)
        
        setNeedsDisplay()
    }
    
    func plotLogData(value:[[Double]], percent: Double) {
        data        = value
        drawPercent = CGFloat(percent)

        setNeedsDisplay()
    }
    
    override func drawRect(rect: CGRect) {
        super.drawRect(rect)
        
        if data[0].count < 2 {
            return
        }
        
        let context = UIGraphicsGetCurrentContext()!
        
        for (index, d) in data.enumerate()   {
            if d.count > 0 {
                drawSingleData(rect, context: context, data: d, color: drawColor[index])
            }
        }
    }
    
    func drawSingleData(rect: CGRect, context: CGContext, data:[Double], color: CGColor)
    {
        let context = UIGraphicsGetCurrentContext()!
        
        // 描画範囲の幅と高さ
        let height = rect.size.height
        let width  = rect.size.width * drawPercent
        // 最大/最小値で
        let dx     = width  / CGFloat(data.count)
        let scaleY = height / CGFloat(maxValue - minValue)
        let offsetY = -1 * scaleY * CGFloat(minValue)
        
        let path = drawPath(data, dx: dx, scaleY: scaleY, offsetY: offsetY)
        
        CGContextAddPath(context, path)
        CGContextSetLineWidth(context, 2)
        CGContextSetStrokeColorWithColor(context, color)
        CGContextStrokePath(context)
    }
    
    func drawPath(data: [Double], dx: CGFloat, scaleY: CGFloat, offsetY: CGFloat) -> CGPath
    {
        let path = CGPathCreateMutable()
        
        // 開始点
        CGPathMoveToPoint(path, nil, 0, CGFloat(data[0]) * scaleY + offsetY)
        var x: CGFloat = 0
        for d in data {
            CGPathAddLineToPoint(path, nil, x, CGFloat(d) * scaleY + offsetY)
            x = x + dx
        }
        
        return path
    }
    
}
