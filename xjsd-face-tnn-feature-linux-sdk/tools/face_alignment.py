import math
import cv2
import numpy as np


standard_faces = np.array([
    [30.2946, 51.6963],
    [65.5318, 51.5014],
    [48.0252, 71.7366],
    [33.5493, 92.3655],
    [62.7299, 92.2041]],dtype=np.float32)


target_faces = np.array([
    [145.26517,161.15865],
    [192.50267,143.54042],
    [184.4904,179.20639],
    [178.36884,209.08125],
    [218.14134,194.67761]],dtype=np.float32)


src_box = np.array([
    [108.7302, 108.8640]
    ,[236.2352, 241.2891]
    ,[1, 1]
],dtype=np.float32)

M = cv2.estimateAffine2D(target_faces,standard_faces)
M2 = M[0]
print('M2',M2)
tanA = M2[1,0] / M2[1,1]
tanB = -M2[0,1] / M2[0,0]
tanAB = (tanA + tanB) / 2
print('tanAB',tanAB)
angle = np.arctan([tanAB]) * 180 / math.pi
print('angle',angle)
center = (-M2[0,2],-M2[1,2])
print('center',center)
M3 = cv2.getRotationMatrix2D(center,-angle[0],1)

print('M3',M3)
print('src_box',src_box)
dst_box = np.dot(M3,src_box)
print('dst_box',dst_box)


img = cv2.imread("test_face04.jpg")    
height, width = img.shape[:2]
src_box = src_box.astype(np.int32)
dst_box = dst_box.astype(np.int32)       
dst = cv2.warpAffine(img,M3,(width,height))
cv2.rectangle(dst, dst_box[0],dst_box[1], (0, 255, 0))

cv2.rectangle(img, src_box[0],src_box[1], (255, 0, 0))
cv2.imshow("img",img)     

cv2.imshow("dst",dst)                                         
cv2.waitKey()
cv2.destroyAllWindows()                                         

