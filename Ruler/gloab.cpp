#include "gloab.h"
/**
 * @brief
 * @param
 * @param
 * @param
 * @return
 */

 /**
 * @brief �����Ƿ��ھ�����
 * @param point Ҫ���ĵ�
 * @param rect Ŀ�����
 * @return true �ھ����ڻ�߽��ϣ����򷵻� false
 */
bool ptInRect(const POINT& point, const RECT& rect) {
    return (point.x >= rect.left && point.x <= rect.right &&
        point.y >= rect.top && point.y <= rect.bottom);
}