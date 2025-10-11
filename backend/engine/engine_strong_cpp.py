# engine_strong_cpp.py
import engine_core

def get_best_move_sp(fen: str, time_ms: int = 2000):
    try:
        result = engine_core.get_best_move_cpp(fen, time_ms)
        best_move_uci = result["bestmove"]
        print(f"Engine chose: {best_move_uci} with eval: {result['cp']}")
        return best_move_uci
    except Exception as e:
        print(f"Error in engine: {e}")
        return "0000"  # resignation
    
def get_best_move(fen: str, time_ms: int = 2000):
    """
    Calls C++ engine
    Returns: {"bestmove": "e2e4", "cp": 35, "mate": 0}
    """
    try:
        result = engine_core.get_best_move_cpp(fen, time_ms)
        return {
            "bestmove": result["bestmove"],
            "cp": result["cp"],
            "mate": result["mate"]
        }
    except Exception as e:
        print(f"Error in engine: {e}")
        return "0000"  # resignation