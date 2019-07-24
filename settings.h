#ifndef SETTINGS_H
#define SETTINGS_H

#endif //SETTINGS_H

#ifndef DIFF_HEURISTIC
#define DIFF_HEURISTIC 0

/*
    Possible values: 
        0 "crh" - clause reduction heuristic, 
        1 "wbh" - weighted binaries heuristic,
        2 "bsh" - backbone search heuristic,
        3 "bsrh" - backbone search renormalized heuristic
*/

#endif // DIFF_HEURISTIC

#ifndef DIRECTION_HEURISTIC
#define DIRECTION_HEURISTIC 0

/*
    Possible values: 
        0 "kcnfs" - selects 1 if x appears more often then -x, 
        1 "march" - selects 1 if diff(x=1) < diff(x=2)
        2 "posit" - selects 0 if x appears more often then -x, 
        3 "satz" - always select 1 first
*/
#endif // DECISION_HEURISTIC

#ifndef PRESELECT_HEURISTIC
#define PRESELECT_HEURISTIC 0

/*
    Possible values: 
        0 propz
        1 clause reduction approximation
*/
#endif // PRESELECT_HEURISTIC

#ifndef AUTARKY_REASONING
#define AUTARKY_REASONING 0

/*
    Possible values: 
        0 don't use autarky_reasoning
        1 use autarky_reasoning
*/

#endif


#ifndef DOUBLE_LOOKAHEAD
#define DOUBLE_LOOKAHEAD 0
/*
    possible values: 
        0 don't use double lookahead
        1 use double look ahead with trigger = 65
        2 use double look ahead with trigger = 0.17 * n
        3 use dynamic look ahead with trigger on start 0.17*n, reduced after each unsuccessfull double look-ahead
        4 use dynamic look ahead with trigger on start 0, updated after each unsuccessfull double look-ahead, reduced slightly after each look-ahead
*/

#endif

#ifndef LOCAL_LEARNING
#define LOCAL_LEARNING 1

/*
    possible values:
        0 don't use local learning
        1 use local learning
*/

#endif