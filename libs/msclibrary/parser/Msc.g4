// Define a grammar called Msc
// Specs from
// https://www.itu.int/rec/dologin_pub.asp?lang=e&id=T-REC-Z.120-201102-I!!PDF-E&type=items

grammar Msc;

tokens {
    NAMEORENV,
    MSCFILE
}

file : mscDocument | mscDefinition;

// 2.3 Comment

textDefinition
    : ('text' | 'TEXT') STRING SEMI
    ;

// 3 Message Sequence Chart document

mscDocument
    : MSCDOCUMENT NAME SEMI (containingClause | mscDocument | mscDefinition)* ENDMSCDOCUMENT SEMI definingMscReference*
    ;

definingMscReference
    : REFERENCE virtuality? NAME
    ;

virtuality
    : VIRTUAL | REDEFINED | FINALIZED
    ;

containingClause
    : (INST instanceItem)+
    ;
instanceItem
    : instanceName=NAME (COLON instanceKind)? (inheritance)? (decomposition)? (dynamicDeclList | SEMI)
    ;
inheritance
    : INHERITS instanceKind
    ;
messageDeclClause
    : (MSG messageDecl SEMI)*
    ;

mscDefinition
    : messageSequenceChart
        |       LANGUAGE NAME SEMI
        |       DATA NAME SEMI
        |       MSG NAME (COMMA NAME)* (COLON LEFTOPEN parameterList RIGHTOPEN)? SEMI
    ;

// 4.1 Message sequence chart

messageSequenceChart
    : virtuality? MSC mscHead mscBody ENDMSC SEMI // TODO add hmsc
    ;

mscHead
    : NAME (mscParameterDecl)? SEMI (mscInstInterface)? mscGateInterface
    ;

mscParameterDecl
    : LEFTOPEN mscParmDeclList RIGHTOPEN
    ;
mscParmDeclList
    : mscParmDeclBlock (SEMI mscParameterDecl)?
    ;
mscParmDeclBlock
    : dataParameterDecl
    | instanceParameterDecl
    | messageParameterDecl
    | timerParameterDecl
    ;

instanceParameterDecl
    : INST instanceParmDeclList
    ;
instanceParmDeclList
    : NAME (COLON instanceKind)? (COMMA instanceParmDeclList)?
    ;

messageParameterDecl
    : MSG messageParmDeclList
    ;
messageParmDeclList
    : messageDeclList
    ;

timerParameterDecl
    : TIMER timerParmDeclList
    ;
timerParmDeclList
    : timerDeclList
    ;

mscInstInterface
    : containingClause
    ;
instanceKind
    : NAME (NAME)*
    ;

mscGateInterface
    : mscGateDef*
    ;
mscGateDef
    : GATE msgGate SEMI // TODO (| methodCallGate | replyGate| createGate | orderGate) SEMI
    ;
msgGate
    : defInGate | defOutGate
    ;

mscBody
    : mscStatement* | instanceDeclStatement*
    ;
mscStatement
    : textDefinition | eventDefinition
    ;

eventDefinition
    : (NAME COLON instanceEventList) | (instanceNameList COLON multiInstanceEventList)
    ;

instanceEventList
    : (startMethod instanceEvent* endMethod)
    | (startSuspension instanceEvent* endSuspension)
    | (startCoregion instanceEvent* endCoregion)
    | (instanceHeadStatement instanceEvent* (instanceEndStatement | stop))
    ;
instanceDeclStatement
    : instanceHeadStatement (orderableEvent | nonOrderableEvent)* (instanceEndStatement | stop)
    ;
instanceEvent
    : orderableEvent
    | nonOrderableEvent
    ;

orderableEvent
    : (LABEL eventName=NAME SEMI)? (messageEvent
//    | incompleteMessageEvent
    | create | timerStatement | action
    | coregion) // Not like in the standard and this does not prevet recursive coregion statements
//    | methodCallEvent | incompleteMethodCallEvent | create | timerStatement | action)
//        (BEFORE orderDestList)? (AFTER orderDestList)?  SEMI (TIME timeDestList end)?
    SEMI
    ;

nonOrderableEvent
    : sharedCondition
//    | sharedMscReference
//    | sharedInlineExpr
    ;

instanceNameList
    : (NAME (COMMA NAME)*) | ALL
    ;

multiInstanceEventList
    : multiInstanceEvent +
    ;
multiInstanceEvent
    : condition // TODO add | mscReference | inlineExpr
    ;

// 4.2 Instance

instanceHeadStatement
    : INSTANCE instanceName=NAME (COLON instanceKind)? (decomposition)? (LEFTOPEN parameterList RIGHTOPEN)? SEMI
    // "(LEFTOPEN parameterList RIGHTOPEN)?" is not in the spec
    ;
instanceEndStatement
    : ENDINSTANCE SEMI
    ;

// 4.3 Message

messageEvent
    : messageOutput | messageInput
    ;
messageOutput
    : OUT msgIdentification (TO inputAddress)? // should not be optional accortdint to spec
    ;
messageInput
    : IN msgIdentification (FROM outputAddress)? // should not be optional accortdint to spec
    ;
incompleteMessageEvent
    : incompleteMessageOutput | incompleteMessageInput
    ;
incompleteMessageOutput
    : OUT msgIdentification TO LOST (inputAddress)?
    ;
incompleteMessageInput
    : IN msgIdentification FROM FOUND (outputAddress)?
    ;
msgIdentification
    : messageName=NAME (COMMA messageInstanceName=NAME)? (LEFTOPEN parameterList RIGHTOPEN)?
    ;
outputAddress
    : (instanceName=NAME | ENV) (VIA gateName=NAME)?
    ;
inputAddress
    : (instanceName=NAME | ENV) (VIA gateName=NAME)?
    ;

// 4.4 Control Flow

methodCallEvent
    : callOut | callIn |replyOut | replyIn
    ;
callOut
    : CALL msgIdentification TO inputAddress
    ;
callIn
    : RECEIVE msgIdentification FROM outputAddress
    ;
replyOut
    : REPLYOUT msgIdentification TO inputAddress
    ;
replyIn
    : REPLYIN msgIdentification FROM outputAddress
    ;
incompleteMethodCallEvent
    : incompleteCallOut | incompleteCallIn | incompleteReplyOut | incompleteReplyIn
    ;
incompleteCallOut
    : CALL msgIdentification TO LOST (inputAddress)?
    ;
incompleteCallIn
    : RECEIVE msgIdentification FROM FOUND (outputAddress)?
    ;
incompleteReplyOut
    : REPLYOUT msgIdentification TO LOST (inputAddress)?
    ;
incompleteReplyIn
    : REPLYIN msgIdentification FROM FOUND (outputAddress)?
    ;
startMethod
    : METHOD SEMI
    ;
endMethod
    : ENDMETHOD SEMI
    ;
startSuspension
    : SUSPENSION SEMI
    ;
endSuspension
    : ENDSUSPENSION SEMI
    ;

// 4.5 Environment and gates

inputDest
    : (LOST (inputAddress)?) | inputAddress
    ;
outputDest
    : (FOUND (outputAddress)?) | outputAddress
    ;
defInGate
    : (gateName=NAME)?  OUT msgIdentification TO inputDest
    ;
defOutGate
    : (gateName=NAME)?  IN msgIdentification FROM outputDest
    ;

// 4.7 Condition

sharedCondition
    : (shared)? conditionIdentification (shared)? SEMI // TODO second shared should be mandatory by spec?
    ;
conditionIdentification
    : CONDITION conditionText
    ;
conditionText
    : conditionNameList
    | (WHEN (conditionNameList | LEFTOPEN expression RIGHTOPEN))
    | OTHERWISE
    ;
conditionNameList
    : NAME (COMMA NAME) *
    ;
shared
    : SHARED (sharedInstanceList | ALL)
    ;
sharedInstanceList
    : NAME (COMMA sharedInstanceList)?
    ;
condition
    : (shared)?  conditionIdentification SEMI
    ;

// 4.8 TIMER

timerStatement
    : startTimer | stopTimer | timeout
    ;

startTimer
    : (STARTTIMER|SET) NAME (COMMA NAME)? duration? (LEFTOPEN parameterList RIGHTOPEN)?
    ;

duration
    : LEFTSQUAREBRACKET durationLimit (COMMA durationLimit)? RIGHTSQUAREBRACKET
    ;

durationLimit
    : NAME /* TODO should be a <expression string> */
    | INF
    ;

stopTimer
    : (STOPTIMER|RESET) NAME (COMMA NAME)?
    ;

timeout
    : TIMEOUT NAME (COMMA NAME)? (LEFTOPEN parameterList RIGHTOPEN)?
    ;

// 4.9 ACTION

action
    : ACTION actionStatement
    ;

actionStatement
    : informalAction | dataStatementList
    ;

informalAction
    : CHARACTERSTRING
    ;

// 4.10 Instance creation

create
    : CREATE NAME (LEFTOPEN parameterList RIGHTOPEN)?
    ;

// 4.11 Instance stop

stop
    : STOP SEMI
    ;

// 5.2 Syntax interface to external data languages

variableString
    : STRING // TODO not correct ?
    ;
typeRefString
    : STRING
    ;
dataDefinitionString
    : STRING
    ;

// 5.4 Declaring DATA

messageDeclList
    : messageDecl (SEMI messageDeclList)?
    ;
messageDecl
    : messageNameList (COLON LEFTOPEN typeRefList RIGHTOPEN)?
    ;
messageNameList
    : NAME (COMMA messageNameList)?
    ;
timerDeclList
    : timerDecl (SEMI timerDeclList)?
    ;
timerDecl
    : timerNameList (duration)? (COLON LEFTOPEN typeRefList RIGHTOPEN)?
    ;
timerNameList
    : NAME (COMMA timerNameList)?
    ;
typeRefList
    : STRING (COMMA typeRefList)?
    ;
dynamicDeclList
    : VARIABLES variableDeclList SEMI
    ;
variableDeclList
    : variableDeclItem (SEMI variableDeclList)?
    ;
variableDeclItem
    : variableList COLON STRING
    ;
variableList
    : variableString (COMMA variableList)?
    ;
dataDefinition
    : (LANGUAGE NAME SEMI)? (wildcardDecl)? (DATA STRING SEMI)?
    ;
wildcardDecl
    : WILDCARDS variableDeclList SEMI
    ;

// 5.5 Static DATA

dataParameterDecl
    : (VARIABLES)?  variableDeclList
    ;
actualDataParameters
    : (VARIABLES)?  actualDataParameterList
    ;
actualDataParameterList
    : expressionString (COMMA actualDataParameterList)?
    ;

// 5.7 Bindings

binding
    : leftBinding | rightBinding
    ;

leftBinding
    : pattern LEFTBINDSYMBOL expression
    ;

rightBinding
    : expression RIGHTBINDSYMBOL pattern
    ;

expression
    : expressionString
    ;

pattern
    : variableString | wildcard
    ;

wildcard
    : NAME // TODO not correct ?
    ;

// 5.8 Data in message and timer parameters

parameterList
    : paramaterDefn (COMMA parameterList)?
    ;

paramaterDefn
    : binding | expression | pattern
    ;

//

expressionString
    : NAME COLON NAME // TODO not correct ?
    ;

variableValue
    : LEFTOPEN NAME RIGHTOPEN
    ;

decomposition
    : DECOMPOSED substructureReference
    ;

substructureReference
    : AS NAME
    ;

// 5.10 DATA IN ACTION boxes

dataStatementList
    : dataStatement (COMMA dataStatementList)?
    ;
dataStatement
    : defineStatement | undefineStatement | binding
    ;
defineStatement
    : DEF variableString
    ;
undefineStatement
    : UNDEF variableString
    ;

// 7.1 Coregion

startCoregion
    : CONCURRENT SEMI
    ;
endCoregion
    : ENDCONCURRENT SEMI
    ;
coregion // this is not as expected in the standard
    : CONCURRENT | ENDCONCURRENT
    ;

/*Keywords*/

ACTION:'action'|'ACTION';
AFTER:'after'|'AFTER';
ALL:'all'|'ALL';
ALT:'alt'|'ALT';
AS:'as'|'AS';
BEFORE:'before'|'BEFORE';
BEGIN:'begin'|'BEGIN';
BOTTOM:'bottom'|'BOTTOM';
CALL:'call'|'CALL';
COMMENT:'comment'|'COMMENT';
CONCURRENT:'concurrent'|'CONCURRENT';
CONDITION:'condition'|'CONDITION';
CONNECT:'connect'|'CONNECT';
CREATE:'create'|'CREATE';
DATA:'data'|'DATA';
DECOMPOSED:'decomposed'|'DECOMPOSED';
DEF:'def'|'DEF';
EMPTY:'empty'|'EMPTY';
END:'end'|'END';
ENDCONCURRENT:'endconcurrent'|'ENDCONCURRENT';
ENDEXPR:'endexpr'|'ENDEXPR';
ENDINSTANCE:'endinstance'|'ENDINSTANCE';
ENDMETHOD:'endmethod'|'ENDMETHOD';
ENDMSC:'endmsc'|'ENDMSC';
ENDMSCDOCUMENT:'endmscdocument'|'ENDMSCDOCUMENT';
ENDSUSPENSION:'endsuspension'|'ENDSUSPENSION';
ENV:'env'|'ENV';
EQUALPAR:'equalpar'|'EQUALPAR';
ESCAPE:'escape'|'ESCAPE';
EXC:'exc'|'EXC';
EXTERNAL:'external'|'EXTERNAL';
FINAL:'final'|'FINAL';
FINALIZED:'finalized'|'FINALIZED';
FOUND:'found'|'FOUND';
FROM:'from'|'FROM';
GATE:'gate'|'GATE';
IN:'in'|'IN';
INF:'inf'|'INF';
INHERITS:'inherits'|'INHERITS';
INITIAL:'initial'|'INITIAL';
INLINE:'inline'|'INLINE';
INST:'inst'|'INST';
INSTANCE:'instance'|'INSTANCE';
INT_BOUNDARY:'int_boundary'|'INT_BOUNDARY';
LABEL:'label'|'LABEL';
LANGUAGE:'language'|'LANGUAGE';
LOOP:'loop'|'LOOP';
LOST:'lost'|'LOST';
METHOD:'method'|'METHOD';
MSC:'msc'|'MSC';
MSCDOCUMENT:'mscdocument'|'MSCDOCUMENT';
MSG:'msg'|'MSG';
NESTABLE:'nestable'|'NESTABLE';
NONNESTABLE:'nonnestable'|'NONNESTABLE';
OFFSET:'offset'|'OFFSET';
OPT:'opt'|'OPT';
ORIGIN:'origin'|'ORIGIN';
OTHERWISE:'otherwise'|'OTHERWISE';
OUT:'out'|'OUT';
PAR:'par'|'PAR';
PARENTHESIS:'parenthesis'|'PARENTHESIS';
RECEIVE:'receive'|'RECEIVE';
REDEFINED:'redefined'|'REDEFINED';
REFERENCE:'reference'|'REFERENCE';
RELATED:'related'|'RELATED';
REPLYIN:'replyin'|'REPLYIN';
REPLYOUT:'replyout'|'REPLYOUT';
RESET : 'reset'|'RESET'; // From the old standard - now it's stoptimer
SEQ:'seq'|'SEQ';
SET : 'set'|'SET'; // From the old standard - now it's startptimer
SHARED:'shared'|'SHARED';
STARTTIMER:'starttimer'|'STARTTIMER';
STOP:'stop'|'STOP';
STOPTIMER:'stoptimer'|'STOPTIMER';
SUSPENSION:'suspension'|'SUSPENSION';
TIME:'time'|'TIME';
TIMEOUT:'timeout'|'TIMEOUT';
TIMER:'timer'|'TIMER';
TO:'to'|'TO';
TOP:'top'|'TOP';
UNDEF:'undef'|'UNDEF';
USING:'using'|'USING';
UTILITIES:'utilities'|'UTILITIES';
VARIABLES:'variables'|'VARIABLES';
VIA:'via'|'VIA';
VIRTUAL:'virtual'|'VIRTUAL';
WHEN:'when'|'WHEN';
WILDCARDS:'wildcards'|'WILDCARDS';
SEMI : ';';
COLON : ':';
COMMA : ',';
MINUS : '-';
LEFTOPEN : '(';
RIGHTOPEN : ')';
LEFTSQUAREBRACKET : '[' ;
RIGHTSQUAREBRACKET : ']' ;


fragment
DECIMALDIGIT : [0-9];

fragment
LETTER : [a-zA-Z];

fragment
NATIONAL : LEFTCURLYBRACKET | VERTICALLINE | RIGHTCURLYBRACKET | OVERLINE | UPWARDARROWHEAD ;

fragment
ALPHANUMERIC : LETTER | DECIMALDIGIT | NATIONAL;

fragment
LEFTCURLYBRACKET : '{' ;

fragment
RIGHTCURLYBRACKET : '}' ;

fragment
VERTICALLINE : '|';

fragment
LEFTCLOSED : LEFTSQUAREBRACKET;

fragment
RIGHTCLOSED : RIGHTSQUAREBRACKET;

fragment
ABSTIMEMARK : '@';

fragment
RELTIMEMARK : '&';

fragment
OVERLINE : '~';

fragment
UPWARDARROWHEAD : '^';

fragment
FULLSTOP : '.';

fragment
UNDERLINE : '_';

fragment
QUALIFIERLEFT : '<<';

fragment
QUALIFIERRIGHT : '>>';

fragment
LEFTANGULARBRACKET : '<';

fragment
RIGHTANGULARBRACKET : '>';

fragment
APOSTROPHE : '\'';

LEFTBINDSYMBOL : ':' '='?; // ':=' TODO should be ':=' only

RIGHTBINDSYMBOL : '=:';

CHARACTERSTRING :
    APOSTROPHE ( ALPHANUMERIC | OTHERCHARACTER | SPECIAL | FULLSTOP | UNDERLINE | ' ' | (APOSTROPHE APOSTROPHE) )* APOSTROPHE ;

fragment
TEXT : ( ALPHANUMERIC | OTHERCHARACTER | SPECIAL | FULLSTOP | UNDERLINE | ' ' | APOSTROPHE )*;

MISC : OTHERCHARACTER | APOSTROPHE;

OTHERCHARACTER : '?' | '%' | '+' | '-' | '!' | '*' | '"' | '='; // exclude '/' as it's used for linebreaks

SPECIAL : ABSTIMEMARK | RELTIMEMARK | LEFTOPEN | RIGHTOPEN | LEFTCLOSED | RIGHTCLOSED | LEFTANGULARBRACKET | RIGHTANGULARBRACKET | '#' | COMMA; //  ';' and ':' were here (ttsiod)

COMPOSITESPECIAL : QUALIFIERLEFT | QUALIFIERRIGHT;

fragment
NOTE : '/*' TEXT '*/';

QUALIFIER : QUALIFIERLEFT /* TEXT */ QUALIFIERRIGHT ;

//NAME : ( LETTER | DECIMALDIGIT | UNDERLINE | FULLSTOP | COMMA )+
//{ if (-1 != $text.IndexOf(',')) { $text = $text.Substring(0, $text.IndexOf(','));}} ;
NAME : ( LETTER | DECIMALDIGIT | UNDERLINE | FULLSTOP | MINUS )+ ;

FILENAME : ( LETTER | DECIMALDIGIT | UNDERLINE | FULLSTOP | MINUS )+  ;

STRING : '"' (ALPHANUMERIC | SPECIAL | FULLSTOP | UNDERLINE)* '"';

COMMENTSKIPED: ('comment \''|'COMMENT \'') TEXT '\'' -> skip;
COMMENTLOST : '/*' .*? '*/' -> channel(2);
WS : [ \t\r\n]+ -> skip ; // skip spaces, tabs, newlines
LB : ('/\r'|'/\n') ->skip; // linebreak
