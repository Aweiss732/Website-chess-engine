
import engine_core2

def get_best_move(fen: str, time_ms: int = 2000):
    """
    Calls C++ engine
    Returns: {"bestmove": "e2e4", "cp": 35, "mate": 0}
    """
    try:
        result = engine_core2.get_best_move_cpp(fen, time_ms)
        best_move_uci = result["bestmove"]
        print(f"Engine chose: {best_move_uci} with eval: {result['cp']}")
        return best_move_uci
    except Exception as e:
        print(f"Error in engine: {e}")
        return "0000"  # resignation