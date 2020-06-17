%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Peter Lakatoš                   %    
% Tic Tac Toe with minimax        %
% Neprocedurálne programovanie    %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   moves

%move(+Token, +CurrentState, -NextState)
%all possible moves
move(Token,[-,A,B,C,D,E,F,G,H],[Token,A,B,C,D,E,F,G,H]).
move(Token,[A,-,B,C,D,E,F,G,H],[A,Token,B,C,D,E,F,G,H]).
move(Token,[A,B,-,C,D,E,F,G,H],[A,B,Token,C,D,E,F,G,H]).
move(Token,[A,B,C,-,D,E,F,G,H],[A,B,C,Token,D,E,F,G,H]).
move(Token,[A,B,C,D,-,E,F,G,H],[A,B,C,D,Token,E,F,G,H]).
move(Token,[A,B,C,D,E,-,F,G,H],[A,B,C,D,E,Token,F,G,H]).
move(Token,[A,B,C,D,E,F,-,G,H],[A,B,C,D,E,F,Token,G,H]).
move(Token,[A,B,C,D,E,F,G,-,H],[A,B,C,D,E,F,G,Token,H]).
move(Token,[A,B,C,D,E,F,G,H,-],[A,B,C,D,E,F,G,H,Token]).

%movePlayer(+N,+CurrentState,-NextState)
%insert players move into CurrentState
movePlayer(N,CurrentState,NextState):-movePlayer(N,CurrentState,[],Nxt), twist(Nxt,NextState),!.

movePlayer(_,[],NextState,NextState).
movePlayer(N,[_|CurrentState],NextState,A):-N1 is N - 1, N1 =:= 0, movePlayer(N1,CurrentState,[x|NextState],A).
movePlayer(N,[H|CurrentState],NextState,A):-N1 is N - 1, movePlayer(N1,CurrentState,[H|NextState],A).

%twist(+Nxt,-NextState)
%twist list
twist(Nxt,NextState):-twist(Nxt,[],NextState).
twist([],A,A).
twist([H|Nxt],S,A):-twist(Nxt,[H|S],A).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   checking

%checkValidMove(+CurrentState, +Counter)
%check if position that player selects is free 
checkValidMove([-|_],1):- !.
checkValidMove([_|CurrentState],N):-N1 is N - 1, checkValidMove(CurrentState, N1).
checkValidMove(_,_):-writeln('That position is already taken'), abort.

%checkWinOrDraw(+CurrentState)
%check if player wins
checkWinOrDraw(CurrentState):- win(x,CurrentState),
                               writePlayground(CurrentState,1), 
                               writeln('I won, this AI sucks :('), 
                               abort.
%check if computer wins
checkWinOrDraw(CurrentState):- win(o,CurrentState), 
                               writePlayground(CurrentState,1), 
                               writeln('Computer won, this AI is so good it may enslave humanity'), 
                               abort. 
%check for draw
checkWinOrDraw(CurrentState):- \+checkFreeSpace(CurrentState), 
                                writePlayground(CurrentState,1), 
                                writeln('Draw ? I guess we`re equally smart'), 
                                abort.
%if none of above is true, then continue in game
checkWinOrDraw(_).

%checkFreeSpace(+CurrentState)
%check if there is some free space in the playground
checkFreeSpace([-|_]).
checkFreeSpace([_|CurrentState]):-checkFreeSpace(CurrentState).

%win(+Token,+CurrentState)
%rows
win(Token,[Token,Token,Token,_,_,_,_,_,_]).
win(Token,[_,_,_,Token,Token,Token,_,_,_]).
win(Token,[_,_,_,_,_,_,Token,Token,Token]).
%diagonals
win(Token,[Token,_,_,_,Token,_,_,_,Token]).
win(Token,[_,_,Token,_,Token,_,Token,_,_]).
%columns
win(Token,[Token,_,_,Token,_,_,Token,_,_]).
win(Token,[_,Token,_,_,Token,_,_,Token,_]).
win(Token,[_,_,Token,_,_,Token,_,_,Token]).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   writing

%just some explanatory notes at the beginnig
writeRules:-
    writeln('Welcome to this Tic Tac Toe game, where you play against computer.'),
    writeln('Your moves will be presented with "x" symbol. You also have an advantage of a first move'),
    writeln('The game board is presented as follows :'),
    writeln('1 2 3'), writeln('4 5 6'), writeln('7 8 9'),
    writeln('To make your move, please write a number in range 1-9').

%writePlayground(+CurrentState, +Counter)
%display current state of the playground
writePlayground(_,10).
writePlayground([H|T],N):- (N mod 3) =:= 0, display(H), nl, N1 is N + 1, writePlayground(T,N1).
writePlayground([H|T],N):- display(H), write(' '), N1 is N + 1, writePlayground(T,N1).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   beginning of the program

play:-writeRules,play([-,-,-,-,-,-,-,-,-]).
%play(+CurrentState)
%main loop
play(CurrentState):-read(N),
                    checkValidMove(CurrentState,N),
                    movePlayer(N,CurrentState,NextState),
                    checkWinOrDraw(NextState),
                    minimax(NextState,NextNextState),
                    checkWinOrDraw(NextNextState),
                    writePlayground(NextNextState,1), !,
                    play(NextNextState).


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   minimax

%generateMoves(+CurrentState, -States, +Token)
%generate all valid moves that can be played by player/computer(depends on Token)
generateMoves(CurrentState,States,Token):- findall(States, move(Token,CurrentState,States),States).

%endCase(+Token, +CurrentState, -Value)
%determine value of ending case
endCase(x,CurrentState,1):-win(x,CurrentState).
endCase(o,CurrentState,-1):-win(o,CurrentState).
endCase(_,CurrentState,0):- \+checkFreeSpace(CurrentState).

%helping predicate to determing whether Token is 'x' or 'o'
x(x).
o(o).

%valueCase(+Token, +CurrentState, -Value)
%end state -> return its value
valueCase(Token,CurrentState,Value):-endCase(Token,CurrentState,Value), !.
%this state isnt end state -> x(Token) = true -> generate all moves that 'o' can make -> as value return minimal value of all those states
valueCase(Token,CurrentState,Value):-x(Token),generateMoves(CurrentState,States,o), minValue(States,_,o,Value).
%similar case as above, but this time 'o' is on move
valueCase(Token,CurrentState,Value):-o(Token),generateMoves(CurrentState,States,x), maxValue(States,_,x,Value).

%getMax(+Value1, +Value2, +State1, +State2, -MaxValue, -MaxState)
getMax(V1,V2,S1,_,V1,S1):-V1 >= V2.
getMax(_,V2,_,S2,V2,S2).

%getMin(+Value1, +Value2, +State1, +State2, -MinValue, -MinState)
getMin(V1,V2,S1,_,V1,S1):-V1 =< V2.
getMin(_,V2,_,S2,V2,S2).

%maxValue(+States, -MaxState, +Token, -MaxValue)
%only 1 state left -> return its value
maxValue([CurrentState],CurrentState,Token,Value):-!, valueCase(Token,CurrentState,Value).
%get value of the first state and then compute and return MaxState and MaxValue of other States -> compare these 2 and return max
maxValue([CurrentState|States],MaxState,Token,Value):- valueCase(Token,CurrentState,Value1),
                                        maxValue(States,MaxState2,Token,Value2),
                                        getMax(Value1,Value2,CurrentState,MaxState2,Value,MaxState), !.

%maxValue(+States, -MinState, +Token, -MinValue)
%similar as maxValue, but now trying to find minimal state
minValue([CurrentState],CurrentState,Token,Value):-!, valueCase(Token,CurrentState,Value).
minValue([CurrentState|States],MinState,Token,Value):- valueCase(Token,CurrentState,Value1),
                                        minValue(States,MinState2,Token,Value2), 
                                        getMin(Value1,Value2,CurrentState,MinState2,Value,MinState), !.

%minimax(+CurrentState, -MaxState)
%generate all moves that 'o' can make -> find state minimize players chance to win and return it
minimax(CurrentState,MinState):- generateMoves(CurrentState,States,o), minValue(States,MinState,o,_).
