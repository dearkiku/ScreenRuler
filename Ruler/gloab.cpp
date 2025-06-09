#include "gloab.h"
/**
 * @brief
 * @param
 * @param
 * @param
 * @return
 */

 /**
 * @brief 检查点是否在矩形内
 * @param point 要检查的点
 * @param rect 目标矩形
 * @return true 在矩形内或边界上，否则返回 false
 */
bool ptInRect(const POINT& point, const RECT& rect) {
    return (point.x >= rect.left && point.x <= rect.right &&
        point.y >= rect.top && point.y <= rect.bottom);
}