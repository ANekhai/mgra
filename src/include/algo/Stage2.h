#ifndef STAGE_2_H_ 
#define STAGE_2_H_ 

/*
canformQ(x,Q) говорит, можно ли из мультицветов мультиребер инцидентных вершине x образовать мультицвет Q.
can incident multiedges of x form multicolor Q (current don't check T-consistent formation)
if return false, then Q cannot be formed
if true - who knows...
*/
template<class graph_t>
bool Algorithm<graph_t>::canformQ(const std::string& x, const Mcolor& Q) const {
    if (x == Infty) {
	return canformQoo;
    }

    // color Q can be formed if some it adjacent multicolors form a partition of Q
    // OR 
    // if every intersection Q \cap QQ = \emptyset or QQ.

    multimularcs_t M = graph.get_adjacent_multiedges_with_split(x);
    for(auto im = M.cbegin(); im != M.cend(); ++im) { 
	Mcolor C(Q, im->second, Mcolor::Intersection); 
	if (C.size() > 0 && C.size() < im->second.size()) { 
		return false;
	} 
    }
    return true;
}

template<class graph_t> 
bool Algorithm<graph_t>::stage2() { 
  bool symplified = false; 
  size_t nr = 0; // number of rearrangements 
  size_t nf = 0; // number of fussions/fissions

  do {
    nr = 0; 
    nf = 0;
	
    for(auto is = graph.begin_vertices(); is != graph.end_vertices(); ++is) {  
      const std::string& x = *is;
      mularcs_t M = graph.get_adjacent_multiedges(x);

      if (graph.is_duplication_vertice(M)) { 
	continue; 
      } 

      multimularcs_t Cx = graph.get_adjacent_multiedges_with_split(x);
	
      for(auto im = M.begin(); im != M.end(); ++im) {
	const std::string& y = im->first;
	const Mcolor& Q = im->second; // color of central edge

	if (y == Infty || Q.size() == graph.size_graph()) { 
	  continue;
	} 

	multimularcs_t Cy = graph.get_adjacent_multiedges_with_split(y);

	if (graph.is_duplication_vertice(graph.get_adjacent_multiedges(y))) { 
	  continue;
	}

	outlog << "Testing mobility of edge " << x << "-" << y << " " << genome_match::mcolor_to_name(Q) << " ";

	// test "mobility" of central edge
	// can it be ever find neighboring edge of the same multicolor
	bool mobilQ = false;

	//here 
	for(auto jc = Cx.cbegin(); jc!= Cx.cend(); ++jc) {
	  if (jc->first != y || (jc->first != Infty && graph.is_duplication_vertice(graph.get_adjacent_multiedges(jc->first)))) { 
		continue; 
	  } // not a cental sub-edge

	  const Mcolor& QQ = jc->second; // color of central sub-edge (QQ is sub-multicolor of Q)
	  if (!member(graph.DiColor, QQ)) { 
		continue;
	  } 


	  for(auto ix = Cx.begin(); ix != Cx.end(); ++ix) { 
	    if (ix->first == y  || (ix->first != Infty && graph.is_duplication_vertice(graph.get_adjacent_multiedges(ix->first)))/*|| ix->first == Infty*/ ) { continue; } 

	    if (canformQ(ix->first, QQ)) {
	      outlog << "MOBIL: " << x << "-" << ix->first << " canForm: " << genome_match::mcolor_to_name(QQ) << std::endl;
	      mobilQ = true;
	      break;
	    }
	  }
	
	  if (mobilQ) { 
	    break;
	  } 
    
	  for(auto iy = Cy.cbegin(); iy != Cy.cend(); ++iy) { 
	    if (iy->first == x || (iy->first != Infty && graph.is_duplication_vertice(graph.get_adjacent_multiedges(iy->first))) /*|| iy->first == Infty*/ ) { continue; } 
	    if (canformQ(iy->first, QQ)) {
	      outlog << "MOBIL: " << y << "-" << iy->first << " canForm: " << genome_match::mcolor_to_name(QQ) << std::endl;
	      mobilQ = true;
	      break;
	    }
	  }

	  if (mobilQ) { 
	    break;
	  } 
	}

	if (mobilQ) continue;

	outlog << "NOT MOBIL" << std::endl;

	bool found = false;

	for (auto ix = Cx.cbegin(); ix != Cx.cend(); ++ix) { 
	  if (ix->first == y || (ix->first != Infty && graph.is_duplication_vertice(graph.get_adjacent_multiedges(ix->first)))) continue;
	  const Mcolor& QQ = ix->second;

	  outlog << " Sub-multiedge " << genome_match::mcolor_to_name(ix->second) << std::endl;

	  vertex_t temp = "";   
	  for(auto iy = Cy.cbegin(); iy != Cy.cend(); ++iy) { 
	    if (iy->first != Infty && graph.is_duplication_vertice(graph.get_adjacent_multiedges(iy->first))) { 
	      continue; 
	    } 
	    if (iy->second == ix->second) { 	
	      temp = iy->first;
	      break; 
	    } 
	  } 

	  if (!member(graph.DiColor, QQ) || temp.empty()) continue; 

	  /*
	    Mcolor C(Q, QQ, Mcolor::Union);
	    // TODO: graph on central edges
	    //if( !MBG.is_T_consistent_color(C) ) continue; // do not create T-consistent color
	    */

	  if(TwoBreak(x, ix->first, y, temp, QQ).apply(graph, true)) {
	    found = true;
	    ++nf;
	  }
	}

	if (found) { break; } 
      }
    } 

    if (nr != 0 || nf != 0) { 
      symplified = true;
    } 
  } while (nr > 0 || nf > 0); 
   
  return symplified;
} 
 
#endif
