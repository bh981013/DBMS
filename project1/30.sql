select e1.before_id as id, p1.name as first_name, p2.name as second_name, p3.name as third_name
from Evolution e1, Evolution e2, Pokemon p1, Pokemon p2, Pokemon p3
where e1.after_id = e2.before_id and p1.id = e1.before_id and p2.id = e1.after_id and p3.id = e2.after_id
order by e1.before_id;