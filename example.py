# coding=utf-8

from fast_match import match, FastMatch

#fmatch = FastMatch("data/disease.txt")
fmatch = FastMatch()
fmatch.insert("乙肝")
fmatch.insert("乙肝大三阳")
fmatch.insert("大三阳")
fmatch.insert("抗病毒治疗")

text = "乙肝大三阳抗病毒治疗需要多长时间？"

pattern = "抗病毒治疗"
pos = match(text, pattern)
if pos >= 0:
    print("Find pattern at position: ", pos)

result = fmatch.parse(text)
print("\nMulti-pattern matching result:")
for instance in result:
    print(instance)

words = fmatch.max_forward_match(text)
print("\nMaximum forward matching word segmentation result:")
print(words)

