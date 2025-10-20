from engine_strong_cpp import get_best_move

fen = "8/1kP5/8/8/8/8/5q2/7K b - - 0 1"

result = get_best_move(fen, time_ms=2000)

print("Best move from engine:", result)
