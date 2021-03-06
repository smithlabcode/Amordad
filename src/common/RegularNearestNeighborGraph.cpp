/*
 *    Part of AMORDAD software
 *
 *    Copyright (C) 2014 University of Southern California,
 *                       Andrew D. Smith and Wenzheng Li
 *
 *    Authors: Andrew D. Smith, Wenzheng Li
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string>
#include <vector>
#include <iostream>

#include "RegularNearestNeighborGraph.hpp"

#include "smithlab_utils.hpp"

using std::vector;
using std::string;
using std::pair;
using std::unordered_map;

using boost::property_map;
using boost::graph_traits;

typedef graph_traits<internal_graph>::edge_iterator edge_itr;
typedef property_map<internal_graph, boost::vertex_index_t>::type IndexMap;


size_t 
RegularNearestNeighborGraph::get_edge_count() const {
  return boost::num_edges(the_graph);
}


size_t 
RegularNearestNeighborGraph::get_vertex_count() const {
  return boost::num_vertices(the_graph) - indices_deleted.size();
}


size_t 
RegularNearestNeighborGraph::convert_name_to_index(const string &name) const {
  unordered_map<string, size_t>::const_iterator x(name_to_index.find(name));
  if (x == name_to_index.end())
    throw SMITHLABException("failed to find index: " + name);
  return x->second;
}


string 
RegularNearestNeighborGraph::convert_index_to_name(const size_t &index) const {
  unordered_map<size_t, string>::const_iterator x(index_to_name.find(index));
  if (x == index_to_name.end())
    throw SMITHLABException("failed to find index: " + smithlab::toa(index));
  return x->second;
}


bool
RegularNearestNeighborGraph::update_vertex(const nng_vertex &u, 
                                           const nng_vertex &v,
                                           const double &w) {
  
  // check to see if edge already exists
  bool already_exists;
  graph_traits<internal_graph>::edge_descriptor the_edge;
  boost::tie(the_edge, already_exists) = boost::edge(u, v, the_graph);
  if (already_exists)
    return false;
  
  if (boost::out_degree(u, the_graph) < maximum_degree) {
    add_edge(u, v, w);
    return true;
  }
  else {
    nng_vertex most_distant;
    double the_distance;
    get_most_distant_neighbor(u, most_distant, the_distance);
    if (the_distance > w) {
      boost::remove_edge(u, most_distant, the_graph);
      boost::add_edge(u, v, w, the_graph);
      return true;
    }
  }
  return false;
}


bool
RegularNearestNeighborGraph::update_vertex(const string &u, 
                                           const string &v,
                                           const double &w) {
  unordered_map<string, size_t>::const_iterator u_idx(name_to_index.find(u));
  if (u_idx == name_to_index.end())
    throw SMITHLABException("cannot update unknown vertex: " + u);
  
  unordered_map<string, size_t>::const_iterator v_idx(name_to_index.find(v));
  if (v_idx == name_to_index.end())
    throw SMITHLABException("cannot update edge to unknown vertex: " + v);
  
  return update_vertex(u_idx->second, v_idx->second, w);
}


void 
RegularNearestNeighborGraph::add_edge(const size_t &u, 
                                      const size_t &v,
                                      const double &w) {
  // check to see if edge already exists
  bool already_exists;
  graph_traits<internal_graph>::edge_descriptor the_edge;
  boost::tie(the_edge, already_exists) = boost::edge(u, v, the_graph);
  
  if (already_exists)
    throw SMITHLABException("attempt to add existing edge");
  
  boost::add_edge(u, v, w, the_graph);
}


void 
RegularNearestNeighborGraph::add_edge(const string &u, const string &v,
                                      const double &w) {
  unordered_map<string, size_t>::const_iterator u_idx(name_to_index.find(u));
  if (u_idx == name_to_index.end())
    throw SMITHLABException("cannot add edge from unknown vertex: " + u);

  unordered_map<string, size_t>::const_iterator v_idx(name_to_index.find(v));
  if (v_idx == name_to_index.end())
    throw SMITHLABException("cannot add edge to unknown vertex: " + v);

  add_edge(u_idx->second, v_idx->second, w);
}


void
RegularNearestNeighborGraph::add_vertex(const string &id) {
  unordered_map<string, size_t>::const_iterator u_idx(name_to_index.find(id));
  if (u_idx != name_to_index.end()) {
    if (!was_deleted(id))
      throw SMITHLABException("cannot add existing vertex: " + id);
    else
      indices_deleted.erase(u_idx->second);
  }
  else {
    const size_t index = boost::num_vertices(the_graph);
    name_to_index[id] = index;
    index_to_name[index] = id;
    boost::add_vertex(the_graph);
  }
}


bool
RegularNearestNeighborGraph::add_vertex_if_new(const string &id) {
  unordered_map<string, size_t>::const_iterator u_idx(name_to_index.find(id));
  if (u_idx == name_to_index.end()) {
    const size_t index = boost::num_vertices(the_graph);
    name_to_index[id] = index;
    index_to_name[index] = id;
    boost::add_vertex(the_graph);
    return true;
  }
  else if(was_deleted(u_idx->second)) {
    indices_deleted.erase(u_idx->second);
    return true;
  }
  else return false;  
}


bool
RegularNearestNeighborGraph::was_deleted(const nng_vertex &u) const {
  if(indices_deleted.find(u) == indices_deleted.end())
    return false;
  else
    return true;
}


bool
RegularNearestNeighborGraph::was_deleted(const std::string &id) const {
  unordered_map<string, size_t>::const_iterator u_idx(name_to_index.find(id));
  if (u_idx == name_to_index.end())
    throw SMITHLABException("no deletion status from unknown vertex: "+ id);
  else
    return was_deleted(u_idx->second);
}
 

void
RegularNearestNeighborGraph::remove_vertex(const nng_vertex &u) {
  indices_deleted.insert(u);
  boost::clear_out_edges(u, the_graph);
}


void
RegularNearestNeighborGraph::remove_vertex(const std::string &id) {
  unordered_map<string, size_t>::const_iterator u_idx(name_to_index.find(id));
  if (u_idx == name_to_index.end())
    throw SMITHLABException("attempt to delete unknown vertex: "+ id);
  else
    remove_vertex(u_idx->second);
}


void
RegularNearestNeighborGraph::add_vertices(const vector<string> &ids) {
  for (size_t i = 0; i < ids.size(); ++i)
    add_vertex(ids[i]);
}


double
RegularNearestNeighborGraph::get_distance(const
                                          graph_traits<internal_graph>::edge_descriptor 
                                          &the_edge) const {
  return boost::get(boost::get(boost::edge_weight, the_graph), the_edge);
}


double
RegularNearestNeighborGraph::get_distance(const nng_vertex &u, 
                                          const nng_vertex &v) const {
  
  graph_traits<internal_graph>::edge_descriptor the_edge;
  bool does_exist = false;
  boost::tie(the_edge, does_exist) = boost::edge(u, v, the_graph);
  
  // an edge with infinite weight means non-existing edge
  if (!does_exist)
    return std::numeric_limits<double>::max();
  
  return get_distance(the_edge);
}
 

double
RegularNearestNeighborGraph::get_distance(const string &u, 
                                          const string &v) const {
  unordered_map<string, size_t>::const_iterator u_idx(name_to_index.find(u));
  if (u_idx == name_to_index.end())
    throw SMITHLABException("attempting to add edge from unknown vertex: " + u);
  
  unordered_map<string, size_t>::const_iterator v_idx(name_to_index.find(v));
  if (v_idx == name_to_index.end())
    throw SMITHLABException("attempting to add edge to unknown vertex: " + v);
  
  return get_distance(u_idx->second, v_idx->second);
}


void
RegularNearestNeighborGraph::get_neighbors(const string &query, 
                                           vector<string> &neighbors,
                                           vector<double> &distances) {
  neighbors.clear();
  distances.clear();
  
  unordered_map<string, size_t>::const_iterator 
    query_itr(name_to_index.find(query));
  assert(query_itr != name_to_index.end());
  
  graph_traits<internal_graph>::out_edge_iterator e_begin, e_end;
  tie(e_begin, e_end) = boost::out_edges(query_itr->second, the_graph);
  
  for (graph_traits<internal_graph>::out_edge_iterator 
         i(e_begin); i != e_end; ++i) {
    
    unordered_map<size_t, string>::const_iterator name = 
      index_to_name.find(boost::target(*i, the_graph));
    if(!was_deleted(name->first)) {
      distances.push_back(get_distance(*i));
      neighbors.push_back(name->second);
    }
    else {
      const nng_vertex u =  boost::source(*i, the_graph);
      const nng_vertex v =  boost::target(*i, the_graph);
      boost::remove_edge(u, v, the_graph);
    }
  }
}


string
RegularNearestNeighborGraph::tostring() const{
  std::ostringstream oss;
  
  oss << graph_name << '\n'
      << maximum_degree << '\n';
  
  // assert(name_to_index.size() == get_vertex_count());

  //removing deleted vertices, mapping old index to new index
  unordered_map<size_t, size_t> old_to_new_index;
  size_t vertices_left = 0;
  for(size_t i = 0; i < get_vertex_count(); i++)
    if(!was_deleted(i)) {
      old_to_new_index[i] = vertices_left;
      vertices_left++;
    }

  //writing id mapping:
  oss << "VERTEX";
  for(unordered_map<string, size_t>::const_iterator i(name_to_index.begin());
      i != name_to_index.end(); ++i)
    if(!was_deleted(i->second))
      oss << '\n' << i->first << '\t' << old_to_new_index[i->second];
  
  //writing edges
  oss << '\n'<< "EDGE";
  graph_traits<internal_graph>::edge_iterator e_i, e_j;
  for (boost::tie(e_i, e_j) = boost::edges(the_graph); e_i != e_j; ++e_i) {
    const nng_vertex u = boost::source(*e_i, the_graph);
    const nng_vertex v = boost::target(*e_i, the_graph);
    if(!was_deleted(u) && !was_deleted(v)) 
      oss << '\n' << old_to_new_index[u] << '\t' << old_to_new_index[v] 
          << '\t' << get_distance(u, v);
  }
  return oss.str();
}


std::istream&
operator>>(std::istream &in, RegularNearestNeighborGraph &nng) {

  // reading the name
  string name;
  getline(in, name);

  string line;
  getline(in, line);
  std::istringstream md_iss(line);
  size_t maximum_degree = 0;
  md_iss >> maximum_degree;
  
  getline(in, line); // this removes "VERTEX"
  
  // read in the vertices
  vector<pair<size_t, string> > vertices;
  while (getline(in, line) && line != "EDGE") {
    std::istringstream iss(line);
    string name;
    size_t number;
    if (!(iss >> name >> number)) 
      throw SMITHLABException("bad vertex line format: " + line);
    
    vertices.push_back(make_pair(number, name));
  }
  
  // add the vertices into the graph (in order)
  sort(vertices.begin(), vertices.end());
  vector<string> ids;
  for (size_t i = 0; i < vertices.size(); ++i) {
    if (i > 0 && vertices[i-1].first != vertices[i].first - 1)
      throw SMITHLABException("non consecutive vertex numbers for: " +
                              vertices[i-1].second + " and " + 
                              vertices[i].second);
    ids.push_back(vertices[i].second);
  }
  nng = RegularNearestNeighborGraph(name, ids, maximum_degree);

  while (getline(in, line)) {
    std::istringstream iss(line);
    size_t a, b;
    double dist = 0.0;
    if (!(iss >> a >> b >> dist))
      throw SMITHLABException("bad edge line format: " + line);
    nng.add_edge(a, b, dist);
  }

  return in;
}


std::ostream&
operator<<(std::ostream &os, const RegularNearestNeighborGraph &nng) {
  return os << nng.tostring();
}


void
RegularNearestNeighborGraph::get_most_distant_neighbor(const nng_vertex &query,
                                                       nng_vertex &result, 
                                                       double &max_dist) {
  graph_traits<internal_graph>::out_edge_iterator e_i, e_j;
  result = query;
  max_dist = 0.0;
  for (tie(e_i, e_j) = boost::out_edges(query, the_graph); e_i != e_j; ++e_i) {

    const nng_vertex v =  boost::target(*e_i, the_graph);
    if(!was_deleted(v)) {
      const double curr_dist = get_distance(*e_i);
      if (curr_dist > max_dist) {
        result = v;
        max_dist = curr_dist;
      }
    }
    else {
      const nng_vertex u =  boost::source(*e_i, the_graph);
      boost::remove_edge(u, v, the_graph);
    }
  }
}
