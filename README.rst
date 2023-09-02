fastMatch: large-scale exact string matching tool
=================================================

fastMatch is a c++ library for large-scale exact string matching, mainly
solving following several problems:

-  single pattern - single text matching

-  single pattern - multiple texts matching

-  multiple patterns - single text matching

-  multiple patterns - multiple texts matching

-  maximum forward matching word segmentation

This library is essentially header-only, and we provide an executable
program ``fastMatch`` for large-scale matching.

Usage
-----

Building using make:

.. code:: shell

   git clone https://github.com/zejunwang1/fastMatch
   cd fastMatch
   make

Multiple texts
~~~~~~~~~~~~~~

.. code:: context

   ./fastMatch --help

   Large-scale Exact String Matching Tool! Usage:
     --input         text string file path
     --pattern       pattern string or pattern string file path
     --num_threads   number of threads
     --num_patterns  number of matching patterns returned
     --fast          enable fast matching mode
     --hit           enable hit matching mode
     --seg           enable maximum forward matching word segmentation
     --N             total number of text strings
     --M             total number of pattern strings
     --help -h       show help information

.. code:: shell

   # match all patterns
   ./fastMatch --input data/query.txt --pattern data/disease.txt

   # return a fixed number of matched patterns
   ./fastMatch --input data/query.txt --pattern data/disease.txt --num_patterns 2

   # search only once for each position of the text string
   ./fastMatch --input data/query.txt --pattern data/disease.txt --fast

   # return only one hit pattern for each text string
   ./fastMatch --input data/query.txt --pattern data/disease.txt --hit

   # maximum forward matching word segmentation
   ./fastMatch --input data/query.txt --pattern data/disease.txt --seg

Some matching results as follows:

.. code:: context

   婴幼儿肺炎咳喘    肺炎
   右眼外伤性白内障右眼完全看不清怎么办？怎么才能怀上宝宝    白内障
   怀孕后痔疮会加重吗    痔疮
   如何治疗焦虑症都是哪些办法    焦虑症
   在检查白癜风要多少钱    白癜风
   宫颈息肉了怎么样治    宫颈息肉    息肉
   子宫内膜息肉手术后注意事项    子宫内膜息肉    息肉
   小儿癫痫要注意哪些饮食呢    小儿癫痫    癫痫
   合肥女性多囊卵巢综合症能怀孕吗    多囊卵巢综合症    囊卵巢综合症
   急性非淋巴白血病m2a这个病该如何治疗这个病该如何治疗    非淋    白血病

Single text
~~~~~~~~~~~

.. code:: cpp

   #include <fastMatch.h>

   int main() {
     string disease_path = "data/disease.txt";
     FastMatch fastMatch(disease_path);
     string query = "乙肝大三阳抗病毒治疗需要多长时间？";
     // Single-pattern matching
     string pattern = "抗病毒治疗";
     int pos = match(query, pattern);
     if (pos >= 0)
       cout << "Find pattern at position: " << pos << endl;
     // Multi-pattern matching
     auto result = fastMatch.parse(query);
     cout << "\nMulti-pattern matching result:\n";
     for (int i = 0; i < result.size(); i++)
       cout << result[i].first << " " << result[i].second << endl;
     // Maximum forward matching word segmentation
     cout << "\nMaximum forward matching word segmentation result:\n";
     auto words = fastMatch.maxForwardMatch(query);
     for (auto& word : words)
       cout << word << " ";
     cout << endl;
     return 0;  
   }

Run ``./singleExample``

.. code:: context

   Find pattern at position: 15

   Multi-pattern matching result:
   乙肝 0
   乙肝大三阳 0
   大三阳 6
   抗病毒治疗 15

   Maximum forward matching word segmentation result:
   乙肝大三阳 抗病毒治疗 需 要 多 长 时 间 ？

Python binding
--------------

Install
~~~~~~~

.. code:: shell

   pip install git+https://github.com/zejunwang1/fastMatch

Alternatively,

.. code:: shell

   git clone https://github.com/zejunwang1/fastMatch
   cd fastMatch
   python setup.py install

.. _single-text-1:

Single text
~~~~~~~~~~~

.. code:: python

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

.. code:: context

   Find pattern at position:  5

   Multi-pattern matching result:
   ('乙肝', 0)
   ('乙肝大三阳', 5)
   ('大三阳', 6)
   ('抗病毒治疗', 15)

   Maximum forward matching word segmentation result:
   SEG[乙肝大三阳, 抗病毒治疗, 需, 要, 多, 长, 时, 间, ？]

License
-------

This project is released under `MIT
license <https://github.com/zejunwang1/fastMatch/blob/main/LICENSE>`__
